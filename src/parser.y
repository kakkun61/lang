%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ast.h"
#include "builtin.h"
#include "create.h"
#include "debug.h"
#include "eval.h"
#include "lang.h"

#ifdef DEBUG
#	define DEBUG_PARSER
#	include "debug.h"
#endif

#ifdef DEBUG_PARSER
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
	If *lang_if;
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
       EQ
       NE
       GR
       GE
       LS
       LE
       FUNC_TOKEN
       OUTER_TOKEN
       INNER_TOKEN
       TRUE_TOKEN
       FALSE_TOKEN
       IF_TOKEN
       ELSE_TOKEN
       ELIF_TOKEN
%type <expression_list> expression_list
%type <expression> expression
                   logical_or_expression
                   logical_and_expression
                   equality_expression
                   relational_expression
                   additive_expression
                   multiplicative_expression
                   unary_expression
                   primary_expression
                   block
                   function_definition_expression
                   function_call_expression
                   if_expression
%type <identifier_list> identifier_list
%type <lang_if> elif_list
                else_kind
                else
                elif
%%
root_expression:
	block {
		#ifdef DEBUG_PARSER
			d("root_expression: block");
		#endif
		set_expression(get_compile_script(), $1);
		// get_compile_script()->expression = $1;
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
	logical_or_expression
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
	| INNER_TOKEN IDENTIFIER_TOKEN ASSIGN_TOKEN expression {
		#ifdef DEBUG_PARSER
			d("expression: INNER_TOKEN IDENTIFIER_TOKEN ASSIGN_TOKEN expression");
		#endif
		$$ = create_inner_assign_expression($2, $4);
	}
	| function_definition_expression
	| function_call_expression
	| if_expression;
logical_or_expression:
	logical_and_expression;
logical_and_expression:
	equality_expression;
equality_expression:
	relational_expression
	| equality_expression EQ relational_expression {
		$$ = create_binary_expression(EQUAL, $1, $3);
	}
	| equality_expression NE relational_expression {
		$$ = create_binary_expression(NOT_EQUAL, $1, $3);
	};
relational_expression:
	additive_expression
	| relational_expression GR additive_expression {
		$$ = create_binary_expression(GRATER, $1, $3);
	}
	| relational_expression GE additive_expression {
		$$ = create_binary_expression(GRATER_EQUAL, $1, $3);
	}
	| relational_expression LS additive_expression {
		$$ = create_binary_expression(LESS, $1, $3);
	}
	| relational_expression LE additive_expression {
		$$ = create_binary_expression(LESS_EQUAL, $1, $3);
	};
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
	| TRUE_TOKEN {
		#ifdef DEBUG_PARSER
			d("primary_expression: TRUE_TOKEN");
		#endif
		$$ = create_value_expression(create_boolean(true));
	}
	| FALSE_TOKEN {
		#ifdef DEBUG_PARSER
			d("primary_expression: FALSE_TOKEN");
		#endif
		$$ = create_value_expression(create_boolean(false));
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
		$$ = create_value_expression(create_foreign_function(NULL, $4));
	}
	| FUNC_TOKEN LP identifier_list RP block {
		#ifdef DEBUG_PARSER
			d("function_definition_expression: FUNC_TOKEN LP identifier_list RP block");
		#endif
		$$ = create_value_expression(create_foreign_function($3, $5));
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
if_expression:
	IF_TOKEN LP expression RP block {
		#ifdef DEBUG_PARSER
			d("if_expression: IF_TOKEN LP expression RP block");
		#endif
		$$ = create_if_expression($3, $5, NULL);
	}
	| IF_TOKEN LP expression RP block else_kind {
		#ifdef DEBUG_PARSER
			d("if_expression: IF_TOKEN LP expression RP block else_kind");
		#endif
		$$ = create_if_expression($3, $5, $6);
	};
else:
	ELSE_TOKEN block {
		#ifdef DEBUG_PARSER
			d("else: ELSE_TOKEN block");
		#endif
		$$ = create_if(NULL, $2, NULL);
	};
else_kind:
	else {
		#ifdef DEBUG_PARSER
			d("else_kind: else");
		#endif
	}
	| elif_list {
		#ifdef DEBUG_PARSER
			d("else_kind: elif_list");
		#endif
	}
	| elif_list else {
		#ifdef DEBUG_PARSER
			d("else_kind: elif_list else");
		#endif
		add_if($1, $2);
		$$ = $1;
	};
elif_list:
	elif {
		#ifdef DEBUG_PARSER
			d("elif_list: elif");
		#endif
	}
	| elif_list elif {
		#ifdef DEBUG_PARSER
			d("elif_list: elif_list elif");
		#endif
		add_if($1, $2);
		$$ = $1;
	};
elif:
	ELIF_TOKEN LP expression RP block {
		#ifdef DEBUG_PARSER
			d("elif: ELIF_TOKEN LP expression RP block");
		#endif
		$$ = create_if($3, $5, NULL);
	};
%%
int yyerror(char const *str) {
	extern char *yytext;
	fprintf(stderr, "Parser error: %s \"%s\"\n", str, yytext);
	return 0;
}

