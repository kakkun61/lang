#pragma once

#include <stdbool.h>

typedef struct Expression_tag Expression;

typedef struct Function_tag Function;

typedef struct IdentifierList_tag {
	char const *identifier;
	struct IdentifierList_tag *next;
} IdentifierList;

typedef enum {
	INTEGER,
	FLOAT,
	BOOLEAN,
	FUNCTION,
	NULL_VALUE
} ValueType;

typedef struct {
	ValueType type;
	union {
		double float_point;
		int integer;
		bool boolean;
		Function *function;
	} u;
} Value;

typedef struct ValueList_tag {
	Value *value;
	struct ValueList_tag *next;
} ValueList;

typedef struct {
	char const *name;
	Value *value;
} Variable;

typedef struct VariableList_tag {
	Variable *variable;
	struct VariableList_tag *next;
} VariableList;

typedef enum {
	LOCAL_VARIABLE,
	OUTER_VARIABLE,
} VariableType;

typedef struct TypedVariableList_tag {
	Variable *variable;
	VariableType type;
	struct TypedVariableList_tag *next;
} TypedVariableList;

typedef struct Context_tag {
	TypedVariableList *variable_list;
	struct Context_tag const *outer;
	VariableList *inner_variable_list;
	Function const *self;
} Context;

typedef enum {
	FOREIGN_FUNCTION,
	NATIVE_FUNCTION,
} FunctionType;

struct Function_tag {
	FunctionType type;
	union {
		struct {
			IdentifierList const *parameter_list;
			Expression const *expression;
			Context const *context;
		} foreign;
		struct {
			Value *(*function)(Context *const, ValueList *const);
		} native;
	} u;
};

typedef enum {
	VALUE,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	MINUS,
	ASSIGN,
	EQUAL,
	NOT_EQUAL,
	GRATER,
	GRATER_EQUAL,
	LESS,
	LESS_EQUAL,
	IDENTIFIER,
	FUNCTION_CALL,
	BLOCK,
	OUTER,
	INNER_ASSIGN,
	IF,
	FOR
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

typedef struct {
	ExpressionList const *initialization;
	ExpressionList const *condition;
	ExpressionList const *step;
	Expression const *body;
} For;

struct Expression_tag {
	ExpressionType type;
	union {
		Value *value;
		Expression const *expression;
		ExpressionPair const *pair;
		Assign *assign;
		char const *identifier;
		ExpressionList const *expression_list;
		FunctionCall *function_call;
		If *lang_if;
		For *lang_for;
	} u;
};

