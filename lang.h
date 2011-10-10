#ifndef LANG_H
#define LANG_H

#include <stdio.h>

typedef struct Expression_tag Expression;

typedef struct Function_tag Function; 

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
} ExpressionType;

typedef enum {
	INTEGER,
	FLOAT,
	FUNCTION,
} ValueType;

typedef struct {
	ValueType type;
	union {
		double float_point;
		int integer;
		Function *function;
	} u;
} Value;

typedef struct ExpressionPair_tag {
	Expression *left;
	Expression *right;
} ExpressionPair;

typedef struct {
	char *variable_name;
	Expression *operand;
} Assign;

typedef struct ExpressionList_tag {
	Expression *expression;
	struct ExpressionList_tag *next;
} ExpressionList;

typedef struct {
	char *identifier;
	ExpressionList *argument_list;
} FunctionCall;

struct Expression_tag {
	ExpressionType type;
	union {
		Value *value;
		ExpressionPair *pair;
		Assign *assign;
		char *identifier;
		ExpressionList *expression_list;
		FunctionCall *function_call;
	} u;
};

typedef enum {
	FOREIGN_FUNCTION,
	NATIVE_FUNCTION,
} FunctionType;

typedef struct IdentifierList_tag {
	char *identifier;
	struct IdentifierList_tag *next;
} IdentifierList;

struct Function_tag {
	FunctionType type;
	union {
		struct {
			IdentifierList *parameter_list;
			ExpressionList *expression_list;
		} foreign;
		struct {
		} native;
	} u;
};

typedef struct {
	char *name;
	Value *value;
} Variable;

typedef struct VariableList_tag {
	Variable *variable;
	struct VariableList_tag *next;
} VariableList;

typedef struct OuterVariableList_tag {
	Variable *variable;
	struct OuterVariableList_tag *next;
} OuterVariableList;

typedef struct Context_tag {
	VariableList *variable_list;
	OuterVariableList *outer_variable_list;
	struct Context_tag *outer;
} Context;

typedef struct {
	Expression *expression;
	Context *global_context;
} Script;

/**
 * @param var 変数名
 * @param start 初期値
 */
#define GET_LAST(var, start) for ((var) = (start); (var)->next; (var) = (var)->next)

/* lang.c */
Expression *create_expression(ExpressionType type);

Value *create_value(ValueType type);

Value *create_float_point(double value);

Value *create_integer(int value);

Value *create_function(IdentifierList *parameter_list, Expression *expression);

ExpressionPair *create_expression_pair(Expression *left, Expression *right);

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right);

Expression *create_value_expression(Value *value);

Expression *create_assign_expression(char *variable_name, Expression *operand);

Expression *create_block_expression(ExpressionList *expression_list);

char *create_identifier(char *identifier);

Expression *create_identifier_expression(char *identifier);

Expression *create_function_call_expression(char *identifier, ExpressionList *argument_list);

Context *create_context(void);

Value *eval(Context *context, Expression *expression);

void print_value(Value *value);

void set_compile_script(Script *script);

Script *get_compile_script(void);

int add_outer_variable(Context *const context, char const *const name);

void add_variable(Context *const context, Variable *const variable);

Variable *get_variable(Context const * const context, char const * const name);

Script *create_script(void);

ExpressionList *create_expression_list(Expression *expression);

void add_expression(ExpressionList *expression_list, Expression *expression);

IdentifierList *create_identifier_list(char *identifier);

void add_identifier(IdentifierList *identifier_list, char *identifier);

void interpret(Script *script);

int compile(FILE *input);

#endif
