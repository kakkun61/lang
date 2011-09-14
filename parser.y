%{
#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

#ifdef DEBUG
#	include "debug.h"
#endif

#define YYDEBUG 1

int yyerror(char const *str);
%}
%union {
	int integer;
	double float_point;
	char *identifier;
	Expression *expression;
}
%token <integer> INTEGER_LITERAL
%token <float_point> FLOAT_POINT_LITERAL
%token <identifier> IDENTIFIER_TOKEN
%token ADD_TOKEN
       SUB_TOKEN
       MUL_TOKEN
       DIV_TOKEN
       LP
       RP
       ASSIGN_TOKEN
       COMMA
       DOT
%type <expression> expression
                   additive_expression
                   multiplicative_expression
                   unary_expression
                   primary_expression
%%
root_expression:
	expression {
		#ifdef DEBUG
			d("root_expression: expression");
		#endif
	}
	| block {
		#ifdef DEBUG
			d("root_expression: block");
		#endif
	};
block:
	expression DOT {
		#ifdef DEBUG
			d("block: expression DOT");
		#endif
	}
	| expression_list expression DOT {
		#ifdef DEBUG
			d("block: expression_list expression DOT");
		#endif
	};
expression_list:
	expression COMMA {
		#ifdef DEBUG
			d("expression_list: expression COMMA");
		#endif
	}
	| expression_list expression {
		#ifdef DEBUG
			d("expression_list: expression_list expression");
		#endif
	};
expression:
	additive_expression
	| IDENTIFIER_TOKEN ASSIGN_TOKEN expression {
		$$ = create_assign_expression($1, $3);
	};
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
	primary_expression;
primary_expression:
	INTEGER_LITERAL {
		#ifdef DEBUG
			d("primary_expression: INTEGER_LITERAL");
		#endif
		$$ = create_value_expression(create_integer($1));
	}
	| FLOAT_POINT_LITERAL {
		$$ = create_value_expression(create_float_point($1));
	}
	| LP expression RP {
		$$ = $2;
	}
	| IDENTIFIER_TOKEN {
		$$ = create_identifier_expression($1);
	};
%%
int yyerror(char const *str) {
	extern char *yytext;
	fprintf(stderr, "Parser error: %s \"%s\"\n", str, yytext);
	return 0;
}

