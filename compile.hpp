#ifndef COMPILE_HPP
#define COMPILE_HPP

extern "C" {
   #include "expr.h"
   #include "parser.h"
   #include "lexer.h"
}

#include "asmjit/src/asmjit/x86.h"
#include <unordered_map>
#include <string>
#include <exception>

using namespace asmjit;

typedef double (*Func)(double);

/* A type for the REPL's symbol table */
class FnTable : public std::unordered_map<std::string, Func> {
public:
   FnTable();
};

typedef std::unordered_map<std::string, double> VarTable;

/* An exception with a method that prints a message */
class ReportingException : public std::exception {
public:
   virtual void report();
};

/* Name resolution failure exception class.
 * Thrown by the compiler when the user tries to use an unknown function.
 */
class NameResFail : public ReportingException {
public:
   std::string name;

   NameResFail(const char *name);

   virtual const char *what();
   
   virtual void report();
};

class ParseError : public ReportingException {
public:
   std::string msg;

   ParseError(char *msg);

   virtual const char *what();

   virtual void report();
};

struct ExecCtx {
   FnTable fnTable;
   VarTable varTable;
};

/* A class to store information for the compiler.
 * Meant to be used for the compilation of a single expression.
 */
class CompCtx {
public:
   const ExecCtx &ectx;
   x86::Compiler cc;
   x86::Xmm y, x;

   JitRuntime &rt;
   CodeHolder &code;

   CompCtx(JitRuntime &rt,
           CodeHolder &code,
           const ExecCtx &ectx);

   /* Starts the recursive compilation of the expression. */
   void conv_expr_rec(const Expr *expr);

   /* Cleans up the compiler and returns the compiled function */
   Func end();

private:
   void conv_unary(const Unary *unary);

   void conv_binary(const Binary *binary);

   void conv_apply(const Apply *apply);

   void conv_var_expr(const char *varname);
};

/* Converts the provided expression into a callable function. */
Func conv_var_expr(const Expr *expr,
                   JitRuntime &rt,
                   const ExecCtx &ectx);

/* Evaluates an expression from a string.
 * Writes the result to the provided double.
 * Returns true if the expression had a result (false if it was a definition).
 */
bool conv_eval_str(JitRuntime &rt,
                   const char *in,
                   ExecCtx &ectx,
                   Expr **expr,
                   double &result,
                   char **funcRes = nullptr,
                   char **varRes = nullptr);

#endif

