%{
   #include "expr.h"
   #include "lexer.h"

   void yyerror(Expr **, char **, char **, char **err, const char *s);


%}

%union {
   double fval;
   Expr *expr;
   char *sval;
}

%parse-param {Expr **root}
%parse-param {char **funcname}
%parse-param {char **varname}
%parse-param {char **err}

%define parse.error detailed

%token NUM VAR ARG FUNC ENDL

%nonassoc '=' '+' '-' '*' '/' '^'

%type <fval> NUM
%type <sval> FUNC VAR
%type <expr> isolate pow cmul neg mul expr stmt

%destructor { free($$); } FUNC VAR

%%

stmt:
     FUNC '=' expr ENDL 
      {
         *funcname = $1;
         *root = $$ = $3;
         YYABORT;
      }
   | VAR '=' expr ENDL
      {
         *varname = $1;
         *root = $$ = $3;
         YYABORT;
      }
   | expr ENDL
      {
         *root = $$ = $1;
         YYABORT;
      }
   ;

isolate:
     VAR
      {
         *root = $$ = new_var_expr($1);
      }
   | ARG
      {
         *root = $$ = new_arg_expr();
      }
   | NUM
      {
         *root = $$ = new_num_expr($1);
      }
   | '(' expr ')'
      {
         *root = $$ = $2;
      }
   | '[' expr ']'
      {
         *root = $$ = new_unary(ABS, $2);
      }
   | FUNC '(' expr ')'
      {
         *root = $$ = new_apply($1, $3);
      }
   | FUNC '[' expr ']'
      {
         *root = $$ = new_apply($1, new_unary(ABS, $3));
      }
   | FUNC '(' ')'
      {
         *root = $$ = new_apply($1, new_num_expr(0));
      }
   ;

pow:
     isolate
      {
         *root = $$ = $1;
      }
   | isolate '^' pow
      {
         *root = $$ = new_binary(POW, $1, $3);
      }
   | isolate '^' '-' pow
      {
         *root = $$ = new_binary(POW, $1, new_unary(NEG, $4));
      }
   ;

cmul:
     pow
      {
         *root = $$ = $1;
      }
   | cmul pow
      {
         *root = $$ = new_binary(MUL, $1, $2);
      }
   ;
   
neg:
     cmul
      {
         *root = $$ = $1;
      }
   | '-' cmul
      {
         *root = $$ = new_unary(NEG, $2);
      }
   ;

mul:
     neg
      {
         *root = $$ = $1;
      }
   | mul '*' neg
      {
         *root = $$ = new_binary(MUL, $1, $3);
      }
   | mul '/' neg
      {
         *root = $$ = new_binary(DIV, $1, $3);
      }
   ;

expr:
     mul
      {
         *root = $$ = $1;
      }
   | expr '+' mul
      {
         *root = $$ = new_binary(ADD, $1, $3);
      }
   | expr '-' mul
      {
         *root = $$ = new_binary(SUB, $1, $3);
      }
   ;

%%

void yyerror(Expr **, char **, char **, char **err, const char *s) {
   *err = strdup(s);
}

