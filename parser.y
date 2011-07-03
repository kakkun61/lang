%{
#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

#ifdef DEBUG
#	include "debug.h"
#endif

#define YYDEBUG 1
%}
%union {
	int integer;
	double float_point;
	Expression *expression;
}
%token <integer> INTEGER_LITERAL
%token <float_point> FLOAT_POINT_LITERAL
%token ADD_TOKEN SUB_TOKEN MUL_TOKEN DIV_TOKEN CR LP RP
%type <expression> expression additive_expression multiplicative_expression unary_expression primary_expression
%%
line_list:
	line {
		#ifdef DEBUG
		d("line_list: line");
		#endif
	}
	| line_list line {
		#ifdef DEBUG
		d("line_list: line_list line");
		#endif
	};
line:
	CR
	| expression CR {
		#ifdef DEBUG
		d("line: expression CR");
		#endif
		add_expression($1);
	};
expression:
	additive_expression;
additive_expression:
	multiplicative_expression
	| additive_expression ADD_TOKEN multiplicative_expression {
		$$ = create_binary_expression(ADD, $1, $3);
	}
	| additive_expression SUB_TOKEN multiplicative_expression {
		$$ = create_binary_expression(SUB, $1, $3);
	};
multiplicative_expression:
	unary_expression
	| multiplicative_expression MUL_TOKEN primary_expression  {
		$$ = create_binary_expression(MUL, $1, $3);
	}
	| multiplicative_expression DIV_TOKEN primary_expression {
		$$ = create_binary_expression(DIV, $1, $3);
	};
unary_expression:
	primary_expression
primary_expression:
	INTEGER_LITERAL {
		$$ = create_value_expression(create_integer($1));
	}
	| FLOAT_POINT_LITERAL {
		$$ = create_value_expression(create_float_point($1));
	}
	| LP expression RP {
		$$ = $2;
	};
%%
int yyerror(char const *str) {
	extern char *yytext;
	fprintf(stderr, "Parser error: \"%s\"\n", yytext);
	return 0;
}

