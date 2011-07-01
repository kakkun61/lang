#ifndef LANG
#define LANG

#include <stdio.h>
#include "debug.h"

//#define DEBUG

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

Expression *create_expression(ExpressionType type) {
	Expression *expr = malloc(sizeof(Expression));
	expr->type = type;
	return expr;
}

Value *create_value(ValueType type) {
	Value *val = malloc(sizeof(Value));
	val->type = type;
	return val;
}

Value *create_float_point(double value) {
	Value *val = create_value(FLOAT);
	val->u.float_point = value;
	return val;
}

Value *create_integer(int value) {
	Value *val = create_value(INTEGER);
	val->u.integer = value;
	return val;
}

ExpressionPair *create_expression_pair(Expression *left, Expression *right) {
	ExpressionPair *pair = malloc(sizeof(ExpressionPair));
	pair->left = left;
	pair->right = right;
	return pair;
}

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right) {
	Expression *expr = create_expression(type);
	expr->u.pair = create_expression_pair(left, right);
	return expr;
}

Expression *create_value_expression(Value *value) {
	Expression *expr = create_expression(VALUE);
	expr->u.value = value;
	return expr;
}

#define BINEXP(op, left, right)\
	switch ((left)->type) {\
	case FLOAT:\
		switch ((right)->type) {\
		case FLOAT:\
			return create_float_point(\
					(left)->u.float_point op (right)->u.float_point\
			);\
		case INTEGER:\
			return create_float_point(\
					(left)->u.float_point op (right)->u.integer\
			);\
		}\
	case INTEGER:\
		switch ((right)->type) {\
		case FLOAT:\
			return create_float_point(\
					(left)->u.integer op (right)->u.float_point\
			);\
		case INTEGER:\
			return create_integer(\
					(left)->u.integer op (right)->u.integer\
			);\
		}\
	}

Value *eval(Expression *expression) {
	switch (expression->type) {
	case VALUE:
		return expression->u.value;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		{
			Value *left = eval(expression->u.pair->left);
			Value *right = eval(expression->u.pair->right);
			switch (expression->type) {
			case ADD:
				BINEXP(+, left, right);
			case SUB:
				BINEXP(-, left, right);
			case MUL:
				BINEXP(*, left, right);
			case DIV:
				BINEXP(/, left, right);
			}
		}
	}
}

#undef BINEXP

void print_value(Value *value) {
	#ifdef DEBUG
	d("print_value");
	#endif
	switch (value->type) {
	case INTEGER:
		printf("%d\n", value->u.integer);
		break;
	case FLOAT:
		printf("%lf\n", value->u.float_point);
		break;
	}
}

static Script *compile_script;

void set_compile_script(Script *script) {
	compile_script = script;
}

Script *get_compile_script() {
	return compile_script;
}

Script *create_script() {
	Script *script = malloc(sizeof(Script));
	script->expression_list = NULL;
	return script;
}

void add_expression(Expression *expression) {
	ExpressionList *el, *crt;
	crt = malloc(sizeof(ExpressionList));
	crt->expression = expression;
	crt->next = NULL;
	if (compile_script->expression_list) {
		for (el = compile_script->expression_list; el->next; el = el->next);
		el->next = crt;
	} else {
		compile_script->expression_list = crt;
	}
}

void interpret(Script *script) {
	ExpressionList *el;
	#ifdef DEBUG
	d("interpret");
	#endif
	for (el = script->expression_list; el; el = el->next) {
		print_value(eval(el->expression));
	}
}

#endif
