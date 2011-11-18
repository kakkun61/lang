#pragma once 
#include "ast.h"

#ifdef DEBUG
#	define DEBUG_EVAL
#endif

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

typedef enum {
	FOREIGN_FUNCTION,
	NATIVE_FUNCTION,
} FunctionType;

typedef struct Function_tag {
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
} Function;

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
} Context;

void add_variable(Context *const context,Variable *const variable,VariableType const type);
Value *eval_if(Context *const context,If const *const lang_if);
void add_inner_variable(Context *const context,Variable *const variable);
Variable *add_outer_variable(Context *const context,char const *const name);
ValueList *create_value_list(Value *value);
Context *create_context(void);
int value2string(char *string,size_t size,const Value *value);
void add_local_variable(Context *const context,Variable *const variable);
Variable *create_variable(char const *const name);
Variable *get_variable(Context const *const context,char const *const name);
Value *eval(Context *const context,Expression const *const expression);
Value *create_native_function(Value *(*function)(Context *const,ValueList *const));
Value *create_native_function(Value *(*function)(Context *const,ValueList *const));
