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
"(" return LP;
")" return RP;
"=" return ASSIGN_TOKEN;
"," return COMMA;
"." return DOT;
";" return SEMICOLON;
func return FUNC_TOKEN;
outer return OUTER_TOKEN;
inner return INNER_TOKEN;
([1-9][0-9]*)|0 {
	int temp;
	sscanf(yytext, "%d", &temp);
	yylval.integer = temp;
	return INTEGER_LITERAL;
}
([1-9][0-9]*\.[0-9]+)|([0-9]\.[0-9]+) {
	double temp;
	sscanf(yytext, "%lf", &temp);
	yylval.float_point = temp;
	return FLOAT_POINT_LITERAL;
}
[A-Za-z_][A-Za-z_0-9]* {
    yylval.identifier = create_identifier(yytext);
    return IDENTIFIER_TOKEN;
}
[ \t\n] ;
. {
	fprintf(stderr, "Lexical error: \"%s\"\n", yytext);
	exit(1);
}
%%

