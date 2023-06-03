extern "C" {
   #include "expr.h"
}

#include "compile.hpp"
#include <vector>
#include <cassert>

void add_fn(JitRuntime &rt,
            const char *in,
            Table &table) {
   Expr *expr;
   char *funcname;
   int errc = 0;
   const char **errv;

   yy_scan_string(in);
   yyparse(&expr, &funcname, &errc, &errv);
   assert(errc == 0);

   Func fn;
   try {
      table[std::string(funcname)] = conv_var_expr(expr, rt, &table);
   }
   catch (NameResFail err) {
      err.report();
   }

   free(funcname);
   yylex_destroy();
   destroy_expr(expr);
}

void test_expr(JitRuntime &rt,
               const char *in,
               Table &table,
               int *ctr, int *fails,
               double expected = 0,
               double delta = 0.001) {
   Expr *expr;
   char *funcname;
   int errc = 0;
   const char **errv;
   double result;

   yy_scan_string(in);
   yyparse(&expr, &funcname, &errc, &errv);
   if (errc != 0) {
      printf("Parser error: %s\n\n", errv[0]);
      ++*fails;
      
      goto done;
   }

   Func fn;
   try {
      fn = conv_var_expr(expr, rt, &table);
   }
   catch (NameResFail err) {
      err.report();
      ++*fails;
      
      goto done; 
   }

   result = (*fn)(0);
   printf("> ");
   print_expr(expr, (FILE *)stdout);
   printf(" = %.4f\n", result);

   if (result < expected - delta || result > expected + delta) {
      printf("FAILED! Expected: %f\n\n", expected);
      ++*fails;
   }
   else {
      printf("Success!\n\n");
   }

done:
   yylex_destroy();
   if (expr != nullptr) {
      destroy_expr(expr);
   }

   ++*ctr;
}

void run_tests() {
   JitRuntime rt;
   Expr *expr;
   char *funcname;

   static const char *functions[] = {
      "F = 2x + 1",
      "G = 2^xF(x)",
      "E = 2.71828",
      "Ln = Log(x)/Log(E())",
      "Pi = 3.1415926535"
   };

   Table table;
   add_default_fns(table);
   for (int i = 0; i < 5; i++) {
      add_fn(rt, functions[i], table);
   }
   
   int ctr = 0, fails = 0;
   test_expr(rt, "F(3)", table, &ctr, &fails, 7);
   test_expr(rt, "G(2)", table, &ctr, &fails, 20);
   test_expr(rt, "G(3)^-F(-1)", table, &ctr, &fails, 56);
   test_expr(rt, "E()^(Ln(5) + Ln(2))", table, &ctr, &fails, 10);
   test_expr(rt, "Cos(Pi())", table, &ctr, &fails, -1);
   test_expr(rt, "2[Sin(3 * Pi()/2)]", table, &ctr, &fails, 2);
   printf("%d tests completed. %d failures. %d successes.\n", ctr, fails, ctr - fails);

   for (auto f: table) {
      rt.release(*f.second);
   }
}
 
