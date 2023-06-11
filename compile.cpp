#include "compile.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>

using namespace asmjit;

FnTable::FnTable() {
   std::unordered_map<std::string, Func> base =
           {{ "Sqrt", &sqrt },
            { "Log",  &log  },
            { "Sin",  &sin  },
            { "Cos",  &cos  },
            { "Tan",  &tan  }};
   
   std::swap(*this, base);
}

void ReportingException::report() {
   printf("Unknown exception thrown.\n\n");
}

NameResFail::NameResFail(const char *_name)
   : name(_name)
{}

const char *NameResFail::what() {
   return "Name resolution failure.";
}

void NameResFail::report() {
   printf("Name could not be resolved.\nName given: %s\n\n", name.c_str());
}

ParseError::ParseError(char *_msg)
   : msg((const char *)_msg)
{
  free(_msg);
}

const char *ParseError::what() {
   return "Error caught from parser.";
}

void ParseError::report() {
   printf("The parser encountered an error: %s\n\n", msg.c_str());
}

CompCtx::CompCtx(JitRuntime &_rt,
                 CodeHolder &_code,
                 const ExecCtx &_ectx)
   : rt(_rt),
     code(_code),
     cc(&_code),
     ectx(_ectx)
{
   x = cc.newXmm();
   y = cc.newXmm();

   FuncNode *f = cc.addFunc(FuncSignatureT<double, double>());
   f->setArg(0, x);
}

void CompCtx::conv_expr_rec(const Expr *expr) {
   switch (expr->type) {
   case UNARY:
      conv_unary(expr->val.unary);
      break;
   case BINARY:
      conv_binary(expr->val.binary);
      break;
   case APPLY:
      conv_apply(expr->val.apply);
      break;
   case VARIABLE:
      conv_var_expr(expr->val.varname);
      break;
   case NUMBER:
      {
         x86::Mem numConst = cc.newDoubleConst(ConstPoolScope::kLocal, expr->val.number);
         cc.movsd(y, numConst);
      }

      break;
   case ARGUMENT:
      cc.movsd(y, x);
      break;
   }
}

void CompCtx::conv_unary(const Unary *unary) {
   conv_expr_rec(unary->inner);
   switch (unary->op) {
   case NEG:
      {
         x86::Mem top = cc.newStack(4, 16);
         cc.movsd(top, y);
         cc.xorps(y, y);
         cc.subsd(y, top);
      }

      break;
   case ABS:
      
      InvokeNode *toAbs;
      cc.invoke(&toAbs,
                (double (*)(double))&abs,
                FuncSignatureT<double, double>());
      toAbs->setArg(0, y);
      toAbs->setRet(0, y);

      break;
   }
}

void CompCtx::conv_binary(const Binary *binary) {
   x86::Mem top = cc.newStack(4, 16);

   conv_expr_rec(binary->rhs);
   cc.movsd(top, y);
   conv_expr_rec(binary->lhs);
   switch (binary->op) {
   case ADD:
      cc.addsd(y, top);
      break;
   case SUB:
      cc.subsd(y, top);
      break;
   case MUL:
      cc.mulsd(y, top);
      break;
   case DIV:
      cc.divsd(y, top);
      break;
   case POW:
      {
         x86::Mem top2 = cc.newStack(4, 16);
         cc.movsd(top2, x);
         cc.movsd(x, top);

         InvokeNode *toPow;
         cc.invoke(&toPow,
                   (double (*)(double, double))&pow,
                   FuncSignatureT<double, double, double>());
         toPow->setArg(0, y);
         toPow->setArg(1, x);
         toPow->setRet(0, y);

         cc.movsd(x, top2);
      }

      break;
   }
}

void CompCtx::conv_apply(const Apply *apply) {
   if (ectx.fnTable.find(apply->funcname) == ectx.fnTable.end()) {
      throw new NameResFail(apply->funcname);
   }

   conv_expr_rec(apply->arg);
   InvokeNode *toFn;
   cc.invoke(&toFn,
             ectx.fnTable.at(apply->funcname),
             FuncSignatureT<double, double>());

   toFn->setArg(0, y);
   toFn->setRet(0, y);
}

void CompCtx::conv_var_expr(const char *varname) {
   if (ectx.varTable.find(varname) == ectx.varTable.end()) {
      throw new NameResFail(varname);
   }

   x86::Mem val = cc.newDoubleConst(ConstPoolScope::kLocal,
                                    ectx.varTable.at(varname));

   cc.movsd(y, val);
}

Func CompCtx::end() {
   cc.ret(y);
   cc.endFunc();
   cc.finalize();

   Func fn;
   Error err = rt.add(&fn, &code);
   if (err) {
      printf("AsmJit failed: %s\n", DebugUtils::errorAsString(err));
      exit(1);
   }

   return fn;
}

Func conv_expr(const Expr *expr,
               JitRuntime &rt,
               const ExecCtx &ectx) {
   CodeHolder code;
   code.init(rt.environment(), rt.cpuFeatures());
   
   CompCtx ctx(rt, code, ectx);
   ctx.conv_expr_rec(expr);
   
   return ctx.end();   
}

bool conv_eval_str(JitRuntime &rt,
                   const char *in,
                   ExecCtx &ectx,
                   Expr **expr,
                   double &result,
                   char **funcRes,
                   char **varRes) {
   char *funcname = nullptr,
        *varname = nullptr;

   char *err = nullptr;
   bool hasResult = false;

   yy_scan_string(in);
   yyparse(expr, &funcname, &varname, &err);
   if (err != nullptr) {
      throw new ParseError(err);
   }
   
   Func fn = conv_expr(*expr, rt, ectx);
   if (funcname != nullptr) {
      ectx.fnTable[funcname] = fn;
      if (funcRes != nullptr) {
         *funcRes = funcname;
      }
   }
   else if (varname != nullptr) {
      ectx.varTable[varname] = fn(0);
      if (varRes != nullptr) {
         *varRes = varname;
      }
   }
   else {
      result = fn(0);
      rt.release(fn);
      hasResult = true;
   }

   if (funcRes == nullptr) {
      free(funcname);
   }
   else if (varRes == nullptr) {
      free(varname);
   }

   yylex_destroy();
   return hasResult;
}

