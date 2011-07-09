#ifndef LANG_H
#define LANG_H

#include <stdio.h>

typedef struct Expression_tag Expression;

typedef enum {
	VALUE,
	ADD,
	SUB,
	MUL,
	DIV,
	ASSIGN,
} ExpressionType;

typedef enum {
	INTEGER,
	FLOAT,
} ValueType;

typedef struct {
	ValueType type;
	union {
		double float_point;
		int integer;
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

struct Expression_tag {
	ExpressionType type;
	union {
		Value *value;
		ExpressionPair *pair;
		Assign *assign;
	} u;
};

typedef struct ExpressionList_tag {
	Expression *expression;
	struct ExpressionList_tag *next;
} ExpressionList;

typedef struct {
	ExpressionList *expression_list;
} Script;

/* lang.c */
Expression *create_expression(ExpressionType type);

Value *create_value(ValueType type);

Value *create_float_point(double value);

Value *create_integer(int value);

ExpressionPair *create_expression_pair(Expression *left, Expression *right);

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right);

Expression *create_value_expression(Value *value);

Expression *create_assign_expression(char *variable_name, Expression *operand);

char *create_identifier(char *identifier);

Value *eval(Expression *expression);

void print_value(Value *value);

void set_compile_script(Script *script);

Script *get_compile_script();

Script *create_script();

void add_expression(Expression *expression);

void interpret(Script *script);

int compile(FILE *input);

#endif
