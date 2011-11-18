#pragma once

typedef struct Expression_tag Expression;

typedef enum {
	VALUE,
	ADD,
	SUB,
	MUL,
	DIV,
	ASSIGN,
	IDENTIFIER,
	FUNCTION_CALL,
	BLOCK,
	OUTER,
	INNER_ASSIGN,
	IF
} ExpressionType;

typedef struct ExpressionPair_tag {
	Expression const *left;
	Expression const *right;
} ExpressionPair;

typedef struct {
	char const *identifier;
	Expression const *operand;
} Assign;

typedef struct ExpressionList_tag {
	Expression const *expression;
	struct ExpressionList_tag *next;
} ExpressionList;

typedef struct {
	char const *identifier;
	ExpressionList const *argument_list;
} FunctionCall;

typedef struct If_tag {
	Expression const *condition;
	Expression const *then;
	struct If_tag *elif;
} If;

struct Expression_tag {
	ExpressionType type;
	union {
		Value *value;
		ExpressionPair const *pair;
		Assign *assign;
		char const *identifier;
		ExpressionList const *expression_list;
		FunctionCall *function_call;
		If *lang_if;
	} u;
};

typedef struct IdentifierList_tag {
	char const *identifier;
	struct IdentifierList_tag *next;
} IdentifierList;

