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
#include "debug.h"

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
       MOD_TOKEN
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
       SELF_TOKEN
       OUTER_TOKEN
       INNER_TOKEN
       TRUE_TOKEN
       FALSE_TOKEN
       IF_TOKEN
       ELSE_TOKEN
       ELIF_TOKEN
       FOR_TOKEN
%type <expression_list> expression_list
                        expression_list_or_empty
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
                   for_expression
%type <identifier_list> identifier_list
%type <lang_if> elif_list
                else_kind
                else
                elif
%%
root_expression:
	block {
		d("root_expression: block");
		set_expression(get_compile_script(), $1);
		// get_compile_script()->expression = $1;
	};
block:
	expression_list DOT {
		d("block: expression_list DOT");
		$$ = create_block_expression($1);
	};
expression_list:
	expression {
		d("expression_list: expression");
		#ifdef TEST
			if ($1 == NULL) {
				d("$1 == NULL");
				exit(1);
			}
		#endif
		$$ = create_expression_list($1);
	}
	| expression_list COMMA expression {
		d("expression_list: expression_list COMMA expression");
		#ifdef TEST
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
		d("expression: IDENTIFIER_TOKEN ASSIGN_TOKEN expression");
		$$ = create_assign_expression($1, $3);
	}
	| OUTER_TOKEN IDENTIFIER_TOKEN {
		d("expression: OUTER_TOKEN IDENTIFIER_TOKEN");
		$$ = create_outer_expression($2);
	}
	| INNER_TOKEN IDENTIFIER_TOKEN ASSIGN_TOKEN expression {
		d("expression: INNER_TOKEN IDENTIFIER_TOKEN ASSIGN_TOKEN expression");
		$$ = create_inner_assign_expression($2, $4);
	}
	| function_definition_expression
	| if_expression
	| for_expression;
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
		d("additive_expression: multiplicative_expression");
		$$ = $1;
	}
	| additive_expression ADD_TOKEN multiplicative_expression {
		d("additive_expression: additive_expression ADD_TOKEN multiplicative_expression");
		$$ = create_binary_expression(ADD, $1, $3);
	}
	| additive_expression SUB_TOKEN multiplicative_expression {
		d("additive_expression: additive_expression SUB_TOKEN multiplicative_expression");
		$$ = create_binary_expression(SUB, $1, $3);
	};
multiplicative_expression:
	unary_expression {
		d("multiplicative_expression: unary_expression");
		$$ = $1;
	}
	| multiplicative_expression MUL_TOKEN primary_expression  {
		d("multiplicative_expression: multiplicative_expression MUL_TOKEN primary_expression");
		$$ = create_binary_expression(MUL, $1, $3);
	}
	| multiplicative_expression DIV_TOKEN primary_expression {
		d("multiplicative_expression: multiplicative_expression DIV_TOKEN primary_expression");
		$$ = create_binary_expression(DIV, $1, $3);
	}
	| multiplicative_expression MOD_TOKEN primary_expression {
		d("multiplicative_expression: multiplicative_expression MOD_TOKEN primary_expression");
		$$ = create_binary_expression(MOD, $1, $3);
	};
unary_expression:
	primary_expression {
		d("unary_expression: primary_expression");
		$$ = $1;
	}
	| SUB_TOKEN unary_expression {
		d("unary_expression: SUB unary_expression");
		$$ = create_minus_expression($2);
	};
primary_expression:
	INTEGER_LITERAL {
		d("primary_expression: INTEGER_LITERAL");
		$$ = create_value_expression(create_integer($1));
	}
	| FLOAT_POINT_LITERAL {
		d("primary_expression: FLOAT_POINT_LITERAL");
		$$ = create_value_expression(create_float_point($1));
	}
	| TRUE_TOKEN {
		d("primary_expression: TRUE_TOKEN");
		$$ = create_value_expression(create_boolean(true));
	}
	| FALSE_TOKEN {
		d("primary_expression: FALSE_TOKEN");
		$$ = create_value_expression(create_boolean(false));
	}
	| LP expression RP {
		d("primary_expression: LP expression RP");
		$$ = $2;
	}
	| IDENTIFIER_TOKEN {
		d("primary_expression: IDENTIFIER_TOKEN");
		$$ = create_identifier_expression($1);
	}
	| function_call_expression;
function_definition_expression:
	FUNC_TOKEN LP RP block {
		d("function_definition_expression: FUNC_TOKEN LP RP block");
		$$ = create_value_expression(create_foreign_function(NULL, $4));
	}
	| FUNC_TOKEN LP identifier_list RP block {
		d("function_definition_expression: FUNC_TOKEN LP identifier_list RP block");
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
		d("function_call_expression: IDENTIFIER_TOKEN LP RP");
		$$ = create_function_call_expression($1, NULL);
	}
	| IDENTIFIER_TOKEN LP expression_list RP {
		d("function_call_expression: IDENTIFIER_TOKEN LP expression_list RP");
		$$ = create_function_call_expression($1, $3);
	}
	| SELF_TOKEN LP RP {
		d("function_call_expression: SELF_TOKEN LP RP");
		$$ = create_function_call_expression(create_identifier(SELF), NULL);
	}
	| SELF_TOKEN LP expression_list RP {
		d("function_call_expression: SELF_TOKEN LP expression_list RP");
		$$ = create_function_call_expression(create_identifier(SELF), $3);
	};
if_expression:
	IF_TOKEN LP expression RP block {
		d("if_expression: IF_TOKEN LP expression RP block");
		$$ = create_if_expression($3, $5, NULL);
	}
	| IF_TOKEN LP expression RP block else_kind {
		d("if_expression: IF_TOKEN LP expression RP block else_kind");
		$$ = create_if_expression($3, $5, $6);
	};
else:
	ELSE_TOKEN block {
		d("else: ELSE_TOKEN block");
		$$ = create_if(NULL, $2, NULL);
	};
else_kind:
	else {
		d("else_kind: else");
	}
	| elif_list {
		d("else_kind: elif_list");
	}
	| elif_list else {
		d("else_kind: elif_list else");
		add_if($1, $2);
		$$ = $1;
	};
elif_list:
	elif {
		d("elif_list: elif");
	}
	| elif_list elif {
		d("elif_list: elif_list elif");
		add_if($1, $2);
		$$ = $1;
	};
elif:
	ELIF_TOKEN LP expression RP block {
		d("elif: ELIF_TOKEN LP expression RP block");
		$$ = create_if($3, $5, NULL);
	};
for_expression:
	FOR_TOKEN LP expression_list_or_empty SEMICOLON
	expression_list_or_empty SEMICOLON expression_list_or_empty RP block {
		d("for_expression: FOR_TOKEN LP expression_list_or_empty SEMICOLON expression_list_or_empty SEMICOLON expression_list_or_empty RP block");
		$$ = create_for_expression($3, $5, $7, $9);
	};
expression_list_or_empty:
	expression_list
	| /* empty */ {
		$$ = NULL;
	}
%%
int yyerror(char const *str) {
	extern char *yytext;
	fprintf(stderr, "Parser error: %s \"%s\"\n", str, yytext);
	return 0;
}

