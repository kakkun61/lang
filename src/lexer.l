%{
#include <stdio.h>
#include "ast.h" // y.tab.h には、ast.h のインクルードがないようなので書いておく
#include "create.h"
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
"==" return EQ;
"!=" return NE;
">=" return GE;
">" return GR;
"<=" return LE;
"<" return LS;
func return FUNC_TOKEN;
outer return OUTER_TOKEN;
inner return INNER_TOKEN;
true return TRUE_TOKEN;
false return FALSE_TOKEN;
if return IF_TOKEN;
else return ELSE_TOKEN;
elif return ELIF_TOKEN;
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

