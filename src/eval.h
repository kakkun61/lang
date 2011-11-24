#pragma once

#include <stdlib.h>
#include "ast.h"

#ifdef DEBUG
#	define DEBUG_EVAL
#endif

#define SELF "self"

void add_variable(Context *const context,Variable *const variable,VariableType const type);
Value *eval_if(Context *const context,If const *const lang_if);
void add_inner_variable(Context *const context,Variable *const variable);
Variable *add_outer_variable(Context *const context,char const *const name);
ValueList *create_value_list(Value *value);
Context *create_context(void);
int value2string(char *string, size_t size,const Value *value);
void add_local_variable(Context *const context,Variable *const variable);
Variable *create_variable(char const *const name);
Variable *get_variable(Context const *const context,char const *const name);
Value *eval(Context *const context,Expression const *const expression);
Value *create_native_function(Value *(*function)(Context *const,ValueList *const));
Value *create_native_function(Value *(*function)(Context *const,ValueList *const));
