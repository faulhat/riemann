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

#include "expr.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

Expr *new_num_expr(double number) {
   Expr *expr = malloc(sizeof(Expr));
   expr->type = NUMBER;
   expr->val.number = number;

   return expr;
}

Expr *new_arg_expr() {
   Expr *expr = malloc(sizeof(Expr));
   expr->type = ARGUMENT;
   
   return expr;
}

Expr *new_unary(UOp op, Expr *inner) {
   Expr *expr = malloc(sizeof(Expr));
   expr->type = UNARY;
   expr->val.unary = malloc(sizeof(Unary));
   expr->val.unary->op = op;
   expr->val.unary->inner = inner;

   return expr;
}

Expr *new_apply(char *funcname, Expr *arg) {
   Expr *expr = malloc(sizeof(Expr));
   expr->type = APPLY;
   expr->val.apply = malloc(sizeof(Apply));
   expr->val.apply->funcname = funcname;
   expr->val.apply->arg = arg;

   return expr;
}

Expr *new_binary(BOp op, Expr *lhs, Expr *rhs) {
   Expr *expr = malloc(sizeof(Expr));
   expr->type = BINARY;
   expr->val.binary = malloc(sizeof(Binary));
   expr->val.binary->op = op;
   expr->val.binary->lhs = lhs;
   expr->val.binary->rhs = rhs;
   
   return expr;
}

Expr *new_var_expr(char *varname) {
   Expr *expr = malloc(sizeof(Expr));
   expr->type = VARIABLE;
   expr->val.varname = varname;

   return expr;
}

void print_expr(const Expr *expr, FILE *to) {
   switch (expr->type) {
   case UNARY:
      switch (expr->val.unary->op) {
      case NEG:
         fprintf(to, "-");
         print_expr(expr->val.unary->inner, to);
         break;
      case ABS:
         fprintf(to, "|");
         print_expr(expr->val.unary->inner, to);
         fprintf(to, "|");
         break;
      }

      break;
   case BINARY:
      fprintf(to, "(");
      print_expr(expr->val.binary->lhs, to);
      fprintf(to, " %c ", expr->val.binary->op);
      print_expr(expr->val.binary->rhs, to);
      fprintf(to, ")");
      break;
   case APPLY:
      fprintf(to, "%s(", expr->val.apply->funcname);
      print_expr(expr->val.apply->arg, to);
      fprintf(to, ")");
      break;
   case NUMBER:
      fprintf(to, "%.2f", expr->val.number);
      break;
   case VARIABLE:
      fprintf(to, "%s", expr->val.varname);
      break;
   case ARGUMENT:
      fprintf(to, "X");
      break;
   }
}

void destroy_expr(Expr *expr) {
   if (expr == NULL) return;   

   switch (expr->type) {
   case UNARY:
      destroy_expr(expr->val.unary->inner);
      free(expr->val.unary);
      break;
   case BINARY:
      destroy_expr(expr->val.binary->lhs);
      destroy_expr(expr->val.binary->rhs);
      free(expr->val.binary);
      break;
   case APPLY:
      free(expr->val.apply->funcname);
      destroy_expr(expr->val.apply->arg);
      free(expr->val.apply);
      break;
   case VARIABLE:
      free(expr->val.varname);
      break;
   default:
      break;
   }

   free(expr);
}

