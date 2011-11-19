#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "create.h"
#include "list-util.h"

#ifdef DEBUG
#	include "debug.h"
#endif

Expression *create_expression(ExpressionType type) {
	#ifdef DEBUG_CREATE
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
	#ifdef DEBUG_CREATE
	d("create_value");
	#endif
	Value *val = malloc(sizeof(Value));
	val->type = type;
	return val;
}

Value *create_float_point(double value) {
	#ifdef DEBUG_CREATE
	d("create_float_point(%lf)", value);
	#endif
	Value *val = create_value(FLOAT);
	val->u.float_point = value;
	return val;
}

Value *create_integer(int value) {
	#ifdef DEBUG_CREATE
	d("create_integer(%d)", value);
	#endif
	Value *val = create_value(INTEGER);
	val->u.integer = value;
	return val;
}

Value *create_boolean(bool value) {
	#ifdef DEBUG_CREATE
	d("create_boolean(%d)", value);
	#endif
	Value *val = create_value(BOOLEAN);
	val->u.boolean = value;
	return val;
}

Function *create_function(FunctionType type) {
	Function *func = malloc(sizeof(Function));
	func->type = type;
	return func;
}

Value *create_foreign_function(IdentifierList *parameter_list, Expression *expression) {
	Value *val = create_value(FUNCTION);
	#ifdef DEBUG_CREATE
		d("create_foreign_function");
	#endif
	#ifdef TEST
		if (expression->type != BLOCK) {
			d("interpreter bug: expression->type != BLOCK");
			exit(1);
		}
	#endif
	val->u.function = create_function(FOREIGN_FUNCTION);
	val->u.function->u.foreign.parameter_list = parameter_list;
	val->u.function->u.foreign.expression = expression;
	return val;
}

ExpressionPair *create_expression_pair(Expression *left, Expression *right) {
	#ifdef DEBUG_CREATE
	d("create_expression_pair");
	#endif
	ExpressionPair *pair = malloc(sizeof(ExpressionPair));
	pair->left = left;
	pair->right = right;
	return pair;
}

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right) {
	#ifdef DEBUG_CREATE
	d("create_binary_expression");
	#endif
	Expression *expr = create_expression(type);
	expr->u.pair = create_expression_pair(left, right);
	return expr;
}

Expression *create_value_expression(Value *value) {
	#ifdef DEBUG_CREATE
	d("create_value_expression");
	#endif
	Expression *expr = create_expression(VALUE);
	expr->u.value = value;
	return expr;
}

Expression *create_assign_sub(ExpressionType const type, char const *const identifier, Expression const *const operand) {
	Expression *expr = create_expression(type);
	expr->u.assign = malloc(sizeof(Assign));
	expr->u.assign->identifier = identifier;
	expr->u.assign->operand = operand;
	return expr;
}

Expression *create_assign_expression(char *identifier, Expression *operand) {
	#ifdef DEBUG_CREATE
	d("create_assign_expression");
	#endif
	return create_assign_sub(ASSIGN, identifier, operand);
}

char *create_identifier(char *identifier) {
    char *new_id;
    new_id = malloc(strlen(identifier) + sizeof('\0'));
    strcpy(new_id, identifier);
    return new_id;
}

Expression *create_identifier_expression(char *identifier) {
	Expression *expr = create_expression(IDENTIFIER);
	expr->u.identifier = identifier;
}

Expression *create_function_call_expression(char *identifier, ExpressionList *argument_list) {
	#ifdef DEBUG_CREATE
		d("create_function_call_expression");
	#endif
	Expression *expr = create_expression(FUNCTION_CALL);
	expr->u.function_call = malloc(sizeof(FunctionCall));
	expr->u.function_call->identifier = identifier;
	expr->u.function_call->argument_list = argument_list;
	return expr;
}

Expression *create_if_expression(Expression const *const condition, Expression const *const then, If *const elif) {
	Expression *expr = create_expression(IF);
	expr->u.lang_if = create_if(condition, then, elif);
	return expr;
}

If *create_if(Expression const *const condition, Expression const *const then, If *const elif) {
	If *if_ = malloc(sizeof(If));
	if_->condition = condition;
	if_->then = then;
	if_->elif = elif;
	return if_;
}

void add_if(If *head, If *elif) {
	If *if_;
	GET_LAST(if_, head, elif);
	if_->elif = elif;
}

ExpressionList *create_expression_list(Expression *expression) {
	ExpressionList *el;
	#ifdef DEBUG_CREATE
		d("create_expression_list");
	#endif
	el = malloc(sizeof(ExpressionList));
	el->expression = expression;
	el->next = NULL;
	return el;
}

void add_expression(ExpressionList *expression_list, Expression *expression) {
	ExpressionList *el;
	#ifdef DEBUG
		if (expression == NULL) {
			d("expression == NULL");
			exit(1);
		}
	#endif
	for (el = expression_list; el->next; el = el->next);
	el->next = create_expression_list(expression);
}

IdentifierList *create_identifier_list(char *identifier) {
	IdentifierList *il;
	#ifdef DEBUG_CREATE
		d("create_identifier_list");
	#endif
	il = malloc(sizeof(IdentifierList));
	il->identifier = identifier;
	il->next = NULL;
	return il;
}

void add_identifier(IdentifierList *identifier_list, char *identifier) {
	IdentifierList *il;
	#ifdef DEBUG_CREATE
		d("add_identifier");
	#endif
	for (il = identifier_list; il->next; il = il->next);
	il->next = create_identifier_list(identifier);
}

Expression *create_block_expression(ExpressionList *expression_list) {
	#ifdef DEBUG_CREATE
	d("create_block_expression");
	#endif
	Expression *expr;
	expr = create_expression(BLOCK);
	expr->u.expression_list = expression_list;
	return expr;
}

Expression *create_outer_expression(char const *const identifier) {
	#ifdef DEBUG_CREATE
		d("create_outer_expression");
	#endif
	Expression *expr = create_expression(OUTER);
	expr->u.identifier = identifier;
	return expr;
}

Expression *create_inner_assign_expression(char const *const identifier, Expression const *const expression) {
	#ifdef DEBUG_CREATE
		d("create_inner_expression");
	#endif
	return create_assign_sub(INNER_ASSIGN, identifier, expression);
}

