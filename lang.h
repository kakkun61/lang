#ifndef LANG_H
#define LANG_H

#include <stdio.h>
#include <stdbool.h>

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
	OUTER,
	INNER_ASSIGN,
	IF
} ExpressionType;

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

typedef struct {
	char const *name;
	Value *value;
} Variable;

typedef enum {
	LOCAL_VARIABLE,
	OUTER_VARIABLE,
} VariableType;

typedef struct TypedVariableList_tag {
	Variable *variable;
	VariableType type;
	struct TypedVariableList_tag *next;
} TypedVariableList;

typedef struct VariableList_tag {
	Variable *variable;
	struct VariableList_tag *next;
} VariableList;

typedef struct Context_tag {
	TypedVariableList *variable_list;
	struct Context_tag const *outer;
	VariableList *inner_variable_list;
} Context;

typedef enum {
	FOREIGN_FUNCTION,
	NATIVE_FUNCTION,
} FunctionType;

typedef struct IdentifierList_tag {
	char const *identifier;
	struct IdentifierList_tag *next;
} IdentifierList;

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

typedef struct {
	Expression *expression;
	Context *global_context;
} Script;

/**
 * @param var 変数名
 * @param start 初期値
 * @param next 構造体の、次を指し示すメンバー
 */
#define GET_LAST(var, start, next) for ((var) = (start); (var)->next; (var) = (var)->next)

/**
 * @param var 変数名（構造体)
 * @param start 初期値
 * @param next 構造体の、次を指し示すメンバー
 */
#define FOR(var, start, next) for ((var) = (start); (var); (var) = (var)->next)

/* lang.c */
Expression *create_expression(ExpressionType type);

Value *create_value(ValueType type);

Value *create_float_point(double value);

Value *create_integer(int value);

Value *create_boolean(bool boolean);

Value *create_foreign_function(IdentifierList *parameter_list, Expression *expression);

Value *create_native_function(Value *(*function)(Context *const context, ValueList *const argument_list));

ExpressionPair *create_expression_pair(Expression *left, Expression *right);

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right);

Expression *create_value_expression(Value *value);

Expression *create_assign_expression(char *identifier, Expression *operand);

Expression *create_block_expression(ExpressionList *expression_list);

char *create_identifier(char *identifier);

Expression *create_identifier_expression(char *identifier);

Expression *create_outer_expression(char const *const identifier);

Expression *create_inner_assign_expression(char const *const identifier, Expression const *const expression);

Expression *create_function_call_expression(char *identifier, ExpressionList *argument_list);

Expression *create_if_expression(Expression const *const condition, Expression const *const then, If *const if_expression);

If *create_if(Expression const *const condition, Expression const *const then, If *const if_expression);

void add_if(If *head, If *elif);

Variable *create_variable(char const *const name);

Context *create_context(void);

Value *eval(Context *const context, Expression const *const expression);

Value *eval_if(Context *const context, If const *const lang_if);

void print_value(Value *value);

void set_compile_script(Script *script);

Script *get_compile_script(void);

void add_inner_variable(Context *const context, Variable *const variable);

Variable *add_outer_variable(Context *const context, char const *const name);

void add_local_variable(Context *const context, Variable *const variable);

void add_variable(Context *const context, Variable *const variable, VariableType const type);

Variable *get_variable(Context const *const context, char const *const name);

Script *create_script(void);

ExpressionList *create_expression_list(Expression *expression);

void add_expression(ExpressionList *expression_list, Expression *expression);

IdentifierList *create_identifier_list(char *identifier);

void add_identifier(IdentifierList *identifier_list, char *identifier);

ValueList *create_value_list(Value *value);

/**
 * size に終端文字は含まない。
 */
int value2string(char *string, size_t size, const Value *value);

void interpret(Script *script);

int compile(FILE *input);

#endif
