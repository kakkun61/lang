%{
#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

#ifdef DEBUG
#	define DEBUG_PARSER
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
	ExpressionList *expression_list;
	IdentifierList *identifier_list;
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
       SEMICOLON
       FUNC_TOKEN
       OUTER_TOKEN
%type <expression_list> expression_list
%type <expression> expression
                   additive_expression
                   multiplicative_expression
                   unary_expression
                   primary_expression
                   block
                   function_definition_expression
                   function_call_expression
%type <identifier_list> identifier_list
%%
root_expression:
	block {
		#ifdef DEBUG_PARSER
			d("root_expression: block");
		#endif
		get_compile_script()->expression = $1;
	};
block:
	expression_list DOT {
		#ifdef DEBUG_PARSER
			d("block: expression_list DOT");
		#endif
		$$ = create_block_expression($1);
	};
expression_list:
	expression {
		#ifdef DEBUG_PARSER
			d("expression_list: expression");
		#endif
		#ifdef DEBUG
			if ($1 == NULL) {
				d("$1 == NULL");
				exit(1);
			}
		#endif
		$$ = create_expression_list($1);
	}
	| expression_list COMMA expression {
		#ifdef DEBUG_PARSER
			d("expression_list: expression_list COMMA expression");
		#endif
		#ifdef DEBUG
			if ($3 == NULL) {
				d("$3 == NULL");
				exit(1);
			}
		#endif
		add_expression($1, $3);
		$$ = $1;
	};
expression:
	additive_expression {
		#ifdef DEBUG_PARSER
			d("expression: additive_expression");
		#endif
		$$ = $1;
	}
	| IDENTIFIER_TOKEN ASSIGN_TOKEN expression {
		#ifdef DEBUG_PARSER
			d("expression: IDENTIFIER_TOKEN ASSIGN_TOKEN expression");
		#endif
		$$ = create_assign_expression($1, $3);
	}
	| OUTER_TOKEN IDENTIFIER_TOKEN {
		#ifdef DEBUG_PARSER
			d("expression: OUTER_TOKEN IDENTIFIER_TOKEN");
		#endif
		$$ = create_outer_expression($2);
	}
	| function_definition_expression
	| function_call_expression;
additive_expression:
	multiplicative_expression {
		#ifdef DEBUG_PARSER
			d("additive_expression: multiplicative_expression");
		#endif
		$$ = $1;
	}
	| additive_expression ADD_TOKEN multiplicative_expression {
		#ifdef DEBUG_PARSER
			d("additive_expression: additive_expression ADD_TOKEN multiplicative_expression");
		#endif
		$$ = create_binary_expression(ADD, $1, $3);
	}
	| additive_expression SUB_TOKEN multiplicative_expression {
		#ifdef DEBUG_PARSER
			d("additive_expression: additive_expression SUB_TOKEN multiplicative_expression");
		#endif
		$$ = create_binary_expression(SUB, $1, $3);
	};
multiplicative_expression:
	unary_expression {
		#ifdef DEBUG_PARSER
			d("multiplicative_expression: unary_expression");
		#endif
		$$ = $1;
	}
	| multiplicative_expression MUL_TOKEN primary_expression  {
		#ifdef DEBUG_PARSER
			d("multiplicative_expression: multiplicative_expression MUL_TOKEN primary_expression");
		#endif
		$$ = create_binary_expression(MUL, $1, $3);
	}
	| multiplicative_expression DIV_TOKEN primary_expression {
		#ifdef DEBUG_PARSER
			d("multiplicative_expression: multiplicative_expression DIV_TOKEN primary_expression");
		#endif
		$$ = create_binary_expression(DIV, $1, $3);
	};
unary_expression:
	primary_expression {
		#ifdef DEBUG_PARSER
			d("unary_expression: primary_expression");
		#endif
		$$ = $1;
	};
primary_expression:
	INTEGER_LITERAL {
		#ifdef DEBUG_PARSER
			d("primary_expression: INTEGER_LITERAL");
		#endif
		$$ = create_value_expression(create_integer($1));
	}
	| FLOAT_POINT_LITERAL {
		#ifdef DEBUG_PARSER
			d("primary_expression: FLOAT_POINT_LITERAL");
		#endif
		$$ = create_value_expression(create_float_point($1));
	}
	| LP expression RP {
		#ifdef DEBUG_PARSER
			d("primary_expression: LP expression RP");
		#endif
		$$ = $2;
	}
	| IDENTIFIER_TOKEN {
		#ifdef DEBUG_PARSER
			d("primary_literal: IDENTIFIER_TOKEN");
		#endif
		$$ = create_identifier_expression($1);
	};
function_definition_expression:
	FUNC_TOKEN LP RP block {
		#ifdef DEBUG_PARSER
			d("function_definition_expression: FUNC_TOKEN LP RP block");
		#endif
		$$ = create_value_expression(create_function(NULL, $4));
	}
	| FUNC_TOKEN LP identifier_list RP block {
		#ifdef DEBUG_PARSER
			d("function_definition_expression: FUNC_TOKEN LP identifier_list RP block");
		#endif
		$$ = create_value_expression(create_function($3, $5));
	};
identifier_list:
	IDENTIFIER_TOKEN {
		$$ = create_identifier_list($1);
	}
	| identifier_list COMMA IDENTIFIER_TOKEN {
		add_identifier($1, $3);
		$$ = $1;
	};
function_call_expression:
	IDENTIFIER_TOKEN LP RP {
		#ifdef DEBUG_PARSER
			d("function_call_expression: IDENTIFIER_TOKEN LP RP");
		#endif
		$$ = create_function_call_expression($1, NULL);
	}
	| IDENTIFIER_TOKEN LP expression_list RP {
		#ifdef DEBUG_PARSER
			d("function_call_expression: IDENTIFIER_TOKEN LP expression_list RP");
		#endif
		$$ = create_function_call_expression($1, $3);
	};
%%
int yyerror(char const *str) {
	extern char *yytext;
	fprintf(stderr, "Parser error: %s \"%s\"\n", str, yytext);
	return 0;
}

