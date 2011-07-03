%{
#include <stdio.h>
#include "lang.h" // y.tab.h には、lang.h のインクルードがないようなので書いておく
#include "y.tab.h"

int yywrap(void) {
	return 1;
}
%}
%%
"+" return ADD_TOKEN;
"-" return SUB_TOKEN;
"*" return MUL_TOKEN;
"/" return DIV_TOKEN;
"\n" return CR;
"(" return LP;
")" return RP;
([1-9][0-9]*)|0 {
	int temp;
	sscanf(yytext, "%d", &temp);
	yylval.integer = temp;
	return INTEGER_LITERAL;
}
([1-9][0-9]*\.[0-9])|([0-9]\.[0-9]*) {
	double temp;
	sscanf(yytext, "%lf", &temp);
	yylval.float_point = temp;
	return FLOAT_POINT_LITERAL;
}
[ \t] ;
. {
	fprintf(stderr, "Lexical error: \"%s\"\n", yytext);
	exit(1);
}
%%
