#pragma once

#include "ast.h"

#ifdef DEBUG
#	define DEBUG_CREATE
#endif

Expression *create_inner_assign_expression(char const *const identifier,Expression const *const expression);
Expression *create_outer_expression(char const *const identifier);
Expression *create_block_expression(ExpressionList *expression_list);
void add_identifier(IdentifierList *identifier_list,char *identifier);
IdentifierList *create_identifier_list(char *identifier);
void add_expression(ExpressionList *expression_list,Expression *expression);
ExpressionList *create_expression_list(Expression *expression);
void add_if(If *head,If *elif);
If *create_if(Expression const *const condition,Expression const *const then,If *const elif);
If *create_if(Expression const *const condition,Expression const *const then,If *const elif);
Expression *create_if_expression(Expression const *const condition,Expression const *const then,If *const elif);
Expression *create_if_expression(Expression const *const condition,Expression const *const then,If *const elif);
Expression *create_function_call_expression(char *identifier,ExpressionList *argument_list);
Expression *create_identifier_expression(char *identifier);
char *create_identifier(char *identifier);
Expression *create_assign_expression(char *identifier,Expression *operand);
Expression *create_value_expression(Value *value);
Expression *create_binary_expression(ExpressionType type,Expression *left,Expression *right);
ExpressionPair *create_expression_pair(Expression *left,Expression *right);
Value *create_foreign_function(IdentifierList *parameter_list,Expression *expression);
Value *create_boolean(bool value);
Value *create_integer(int value);
Value *create_float_point(double value);
Value *create_value(ValueType type);
Expression *create_expression(ExpressionType type);
Function *create_function(FunctionType type);
Expression *create_minus_expression(Expression const *operand);
