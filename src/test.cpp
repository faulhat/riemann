/*
 * This file is part of the Riemann Project.
 * Developed by Tom Faulhaber for personal use.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 */

extern "C" {
   #include "expr.h"
}

#include "compile.hpp"
#include <vector>
#include <cassert>
#include <map>

/**
 * Tests to make sure an expression produces the expected result.
 *
 * @param rt The asmjit runtime
 * @param in The expression in string form
 * @param ectx The relevant symbol tables
 * @param ctr The counter for how many tests have been run
 * @param fails The counter for how many tests have failed
 * @param expected The expected value of the expression
 * @param delta Error tolerance
 */
void test_expr(JitRuntime &rt,
               const char *in,
               ExecCtx &ectx,
               int *ctr, int *fails,
               double expected = 0,
               double delta = 0.001) {
   Expr *expr = nullptr;
   double result;
   char *funcname = nullptr, *varname = nullptr;
   try {
      if (conv_eval_str(rt, in, ectx, &expr, result, &funcname, &varname)) {
         printf("> ");
         print_expr(expr, (FILE *)stdout);
         printf(" = %.2f\n", result);
         if (result < expected - delta || result > expected + delta) {
            printf("FAILED! Expected: %f\n\n", expected);
            ++*fails;
         }
         else {
            printf("Success!\n\n");
         }

         ++*ctr;
      }
      else if (funcname != nullptr) {
         printf("%s(x) = ", funcname);
         print_expr(expr, (FILE *)stdout);
         printf("\n\n");
         free(funcname);
      }
      else if (varname != nullptr) {
         printf("%s = ", varname);
         print_expr(expr, (FILE *)stdout);
         printf("\n\n");
         free(varname);
      }
   }
   catch (ReportingException *e) {
      e->report();
      free(e);
   }

   destroy_expr(expr);
}

/**
 * Tests if two expressions are equal
 *
 * @param rt The asmjit runtime
 * @param in_a The first expression in string form
 * @param in_b The second expression in string form
 * @param ectx The relevant symbol tables
 * @param ctr The counter for how many tests have been run
 * @param fails The counter for how many tests have failed
 * @param delta Error tolerance
 */
void test_equal(JitRuntime &rt,
                const char *in_a,
                const char *in_b,
                ExecCtx &ectx,
                int *ctr, int *fails,
                double delta = 0.001) {
   Expr *expr_a = nullptr, *expr_b = nullptr;
   double res_a, res_b;
   char *funcname = nullptr, *varname = nullptr;
   try {
      conv_eval_str(rt, in_a, ectx, &expr_a, res_a, &funcname, &varname);
      assert(funcname == nullptr && varname == nullptr);
      printf("> ");
      print_expr(expr_a, (FILE *)stdout);
      printf(" = %.2f\n", res_a);
      
      conv_eval_str(rt, in_b, ectx, &expr_b, res_b, &funcname, &varname);
      assert(funcname == nullptr && varname == nullptr);
      printf("> ");
      print_expr(expr_b, (FILE *)stdout);
      printf(" = %.2f\n", res_b);

      if (res_a < res_b - delta || res_a > res_b + delta) {
         printf("FAILED! Above results should have been equal.\n\n");
         ++*fails;
      }
      else {
         printf("Success!\n\n");
      }
      
      ++*ctr;
   }
   catch (ReportingException *e) {
      e->report();
      free(e);
   }
   
   destroy_expr(expr_a);
   destroy_expr(expr_b);
}  

/**
 * Runs a series of tests for the expression evaluation program.
 *
 * @return The number of tests which failed.
 */
int run_tests() {
   JitRuntime rt;

   std::vector<const char *> functions = {
      "F = 2x + 1",
      "G = 2^x * F(x)",
      "Ln = Log(x)/Log(e)",
      "One = Sin(x)^2 + Cos(x)^2",
      "TaylorSin = x - (x^3/6) + (x^5/120)"
   };

   ExecCtx ectx;
   int ctr = 0, fails = 0;
   for (auto fn: functions) {
      test_expr(rt, fn, ectx, &ctr, &fails); 
   }
  
   std::map<const char *, double> tests = {
      { "F(3)", 7 },
      { "G(2)", 20 },
      { "G(3)^-F(-1)", 56 },
      { "e^(Ln(5) + Ln(2))", 10 },
      { "Cos(pi)", -1 },
      { "2[Sin(3 * pi/2)]", 2 },
      { "One(1231.1233241)", 1 }};
 
   for (auto t: tests) {
      test_expr(rt, t.first, ectx, &ctr, &fails, t.second);
   }

   std::map<const char *, const char *> eqtests = {
      { "F(3)", "F(3)" },
      { "2/Sqrt(2)", "Sqrt(2)" },
      { "Cos(2pi)", "Cos(0)" },
      { "Cos(2)^2", "1 - Sin(2)^2" },
      { "Sqrt(5)", "5^(1/2)" },
      { "Sin(0.2)", "TaylorSin(0.2)" }};

   for (auto t: eqtests) {
      test_equal(rt, t.first, t.second, ectx, &ctr, &fails);
   }

   printf("%d tests completed. %d failures. %d successes.\n", ctr, fails, ctr - fails);

   for (auto f: ectx.fnTable) {
      rt.release(*f.second);
   }

   return fails;
}
 
