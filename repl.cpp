#include "compile.hpp"

#include <cstdio>
#include <iostream>
#include <sstream>

using namespace asmjit;

void repl() {
   JitRuntime rt;
   
   Expr *expr;
   std::stringstream linestream;
   std::string line;
   char next;

   ExecCtx ectx;
   Func fn;
   double result;
   while (true) {
      linestream.str("");

      do {
         next = getchar();
         if (next == EOF) {
            exit(0);
         }

         linestream << next;
      } while (next != '\n' && next != EOF);

      line = linestream.str();
      bool input_null = true;
      for (char ch: line) {
         if (!isspace(ch)) {
            input_null = false;
            break;
         }
      }

      if (input_null) {
         continue;
      }

      expr = nullptr;
      try {
         if (conv_eval_str(rt, line.c_str(), ectx, &expr, result)) {
            printf("> ");
            print_expr(expr, (FILE *)stdout);
            printf(" = %.2f\n", result);
         }

         printf("\n");
      }
      catch (ReportingException *e) {
         e->report();
      }

      if (expr != nullptr) {
        destroy_expr(expr);
      }
   }

   for (auto f: ectx.fnTable) {
      rt.release(*f.second);
   }
}
