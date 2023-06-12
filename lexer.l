%{
   #include "expr.h"
   #include "parser.h"
   #include <math.h>
%}

%%

[0-9]+|[0-9]*\.[0-9]+ {
   yylval.fval = atof(yytext);
   return NUM;
}

[-+*/[\]()^=] return *yytext;

x return ARG;

pi {
   yylval.fval = M_PI;
   return NUM;
}

e {
   yylval.fval = M_E;
   return NUM;
}

[a-z][0-9_]* {
   yylval.sval = strdup(yytext);
   return VAR;
}

[A-Z][A-Za-z0-9_]* {
   yylval.sval = strdup(yytext);
   return FUNC;
}

[ \t] ;

[\n\r] return ENDL;

<<EOF>> return ENDL;

. {
   printf("WARNING: Unrecognized char ignored: %s\n", yytext);
}

%%

int yywrap() {
   return 1;
}

