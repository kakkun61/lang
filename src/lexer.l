%{
#include <stdio.h>
#include "ast.h" // y.tab.h には、ast.h のインクルードがないようなので書いておく
#include "create.h"
#include "y.tab.h"

int yywrap(void) {
	return 1;
}
%}

%start LINE_COMMENT BLOCK_COMMENT

%%
<INITIAL>"+" return ADD_TOKEN;
<INITIAL>"-" return SUB_TOKEN;
<INITIAL>"*" return MUL_TOKEN;
<INITIAL>"/" return DIV_TOKEN;
<INITIAL>"%" return MOD_TOKEN;
<INITIAL>"(" return LP;
<INITIAL>")" return RP;
<INITIAL>"=" return ASSIGN_TOKEN;
<INITIAL>"," return COMMA;
<INITIAL>"." return DOT;
<INITIAL>";" return SEMICOLON;
<INITIAL>"==" return EQ;
<INITIAL>"!=" return NE;
<INITIAL>">=" return GE;
<INITIAL>">" return GR;
<INITIAL>"<=" return LE;
<INITIAL>"<" return LS;
<INITIAL>func return FUNC_TOKEN;
<INITIAL>self return SELF_TOKEN;
<INITIAL>outer return OUTER_TOKEN;
<INITIAL>inner return INNER_TOKEN;
<INITIAL>true return TRUE_TOKEN;
<INITIAL>false return FALSE_TOKEN;
<INITIAL>if return IF_TOKEN;
<INITIAL>else return ELSE_TOKEN;
<INITIAL>elif return ELIF_TOKEN;
<INITIAL>for return FOR_TOKEN;
<INITIAL>([1-9][0-9]*)|0 {
	int temp;
	sscanf(yytext, "%d", &temp);
	yylval.integer = temp;
	return INTEGER_LITERAL;
}
<INITIAL>([1-9][0-9]*\.[0-9]+)|([0-9]\.[0-9]+) {
	double temp;
	sscanf(yytext, "%lf", &temp);
	yylval.float_point = temp;
	return FLOAT_POINT_LITERAL;
}
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    yylval.identifier = create_identifier(yytext);
    return IDENTIFIER_TOKEN;
}
<INITIAL>[ \t\n] ;
<INITIAL>"//" BEGIN LINE_COMMENT;
<INITIAL>"/*" BEGIN BLOCK_COMMENT;
<INITIAL>. {
	fprintf(stderr, "Lexical error: \"%s\"\n", yytext);
	exit(1);
}
<LINE_COMMENT>\n BEGIN INITIAL;
<LINE_COMMENT>. ;
<BLOCK_COMMENT>"*/" BEGIN INITIAL;
<BLOCK_COMMENT>. ;
%%

