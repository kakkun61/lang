#include <stdlib.h>
#include <stdio.h>
#include "lang.h"

#ifdef DEBUG
#	include "debug.h"
#endif

Expression *create_expression(ExpressionType type) {
	#ifdef DEBUG
	d("create_expression");
	#endif
	Expression *expr = malloc(sizeof(Expression));
	if (!expr) {
		fprintf(stderr, "Memory allocation error\n");
		exit(1);
	}
	expr->type = type;
	return expr;
}

Value *create_value(ValueType type) {
	#ifdef DEBUG
	d("create_value");
	#endif
	Value *val = malloc(sizeof(Value));
	val->type = type;
	return val;
}

Value *create_float_point(double value) {
	#ifdef DEBUG
	d("create_float_point(%lf)", value);
	#endif
	Value *val = create_value(FLOAT);
	val->u.float_point = value;
	return val;
}

Value *create_integer(int value) {
	#ifdef DEBUG
	d("create_integer(%d)", value);
	#endif
	Value *val = create_value(INTEGER);
	val->u.integer = value;
	return val;
}

ExpressionPair *create_expression_pair(Expression *left, Expression *right) {
	#ifdef DEBUG
	d("create_expression_pair");
	#endif
	ExpressionPair *pair = malloc(sizeof(ExpressionPair));
	pair->left = left;
	pair->right = right;
	return pair;
}

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right) {
	#ifdef DEBUG
	d("create_binary_expression");
	#endif
	Expression *expr = create_expression(type);
	expr->u.pair = create_expression_pair(left, right);
	return expr;
}

Expression *create_value_expression(Value *value) {
	#ifdef DEBUG
	d("create_value_expression");
	#endif
	Expression *expr = create_expression(VALUE);
	expr->u.value = value;
	return expr;
}

Expression *create_assign_expression(char *variable_name, Expression *operand) {
	#ifdef DEBUG
	d("create_assign_expression");
	#endif
	Expression *expr = create_expression(ASSIGN);
	expr->u.assign = malloc(sizeof(Assign));
	expr->u.assign->variable_name = variable_name;
	expr->u.assign->operand = operand;
	return expr;
}

#define BINEXP(op, left, right)\
	({\
		Value *tmp;\
		switch ((left)->type) {\
		case FLOAT:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_float_point(\
						(left)->u.float_point op (right)->u.float_point\
				);\
			case INTEGER:\
				tmp = create_float_point(\
						(left)->u.float_point op (right)->u.integer\
				);\
			}\
		case INTEGER:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_float_point(\
						(left)->u.integer op (right)->u.float_point\
				);\
			case INTEGER:\
				tmp = create_integer(\
						(left)->u.integer op (right)->u.integer\
				);\
			}\
		}\
		tmp;\
	})

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
				return BINEXP(+, left, right);
			case SUB:
				return BINEXP(-, left, right);
			case MUL:
				return BINEXP(*, left, right);
			case DIV:
				return BINEXP(/, left, right);
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
	#ifdef DEBUG
	d("interpret");
	#endif
	ExpressionList *el;
	for (el = script->expression_list; el; el = el->next) {
		print_value(eval(el->expression));
	}
}

int compile(FILE *input) {
	extern int yyparse(void);
	extern FILE *yyin;

	yyin = input;
	return yyparse();
}

