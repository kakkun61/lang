#pragma once

#include <stdlib.h>
#include "ast.h"

typedef enum {
	NORMAL_RETURN,
	RETURN_RETURN,
	BREAK_RETURN,
	CONTINUE_RETURN,
} ReturnType;

typedef struct {
	ReturnType type;
	Value *value;
} Return;

#define SELF "self"

void add_inner_variable(Context *const context,Variable *const variable);
Variable *add_outer_variable(Context *const context,char const *const name);
ValueList *create_value_list(Value *value);
Context *create_context(ContextType type);
int value2string(char *string, size_t size,const Value *value);
void add_local_variable(Context *const context,Variable *const variable);
Variable *create_variable(char const *const name);
Variable *get_variable(Context const *const context,char const *const name);
Return *eval(Context *const context,Expression const *const expression);
Value *create_native_function(Value *(*function)(Context *const,ValueList *const));
Return *create_return(ReturnType const type, Value *const value);
