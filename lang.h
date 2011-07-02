#ifndef LANG_H
#define LANG_H

#include <stdio.h>

typedef struct ExpressionS Expression;

typedef enum {
	VALUE,
	ADD,
	SUB,
	MUL,
	DIV
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

typedef struct ExpressionPairS {
	Expression *left;
	Expression *right;
} ExpressionPair;

struct ExpressionS {
	ExpressionType type;
	union {
		Value *value;
		ExpressionPair *pair;
	} u;
};

typedef struct ExpressionListS {
	Expression *expression;
	struct ExpressionListS *next;
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

Value *eval(Expression *expression);

void print_value(Value *value);

void set_compile_script(Script *script);

Script *get_compile_script();

Script *create_script();

void add_expression(Expression *expression);

void interpret(Script *script);

int compile(FILE *input);

#endif
