#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lang.h"
#include "builtin.h"

#ifdef TEST
#	include "debug.h"
#endif

#ifdef DEBUG
#	define DEBUG_LANG
#	include "debug.h"
#	ifndef TEST
#		define TEST
#	endif
#endif

static void prepare_builtin_function(Context *const context);
static Function *create_function(FunctionType type);

Expression *create_expression(ExpressionType type) {
	#ifdef DEBUG_LANG
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
	#ifdef DEBUG_LANG
	d("create_value");
	#endif
	Value *val = malloc(sizeof(Value));
	val->type = type;
	return val;
}

Value *create_float_point(double value) {
	#ifdef DEBUG_LANG
	d("create_float_point(%lf)", value);
	#endif
	Value *val = create_value(FLOAT);
	val->u.float_point = value;
	return val;
}

Value *create_integer(int value) {
	#ifdef DEBUG_LANG
	d("create_integer(%d)", value);
	#endif
	Value *val = create_value(INTEGER);
	val->u.integer = value;
	return val;
}

static Function *create_function(FunctionType type) {
	Function *func = malloc(sizeof(Function));
	func->type = type;
	return func;
}

Value *create_foreign_function(IdentifierList *parameter_list, Expression *expression) {
	Value *val = create_value(FUNCTION);
	#ifdef DEBUG_LANG
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

Value *create_native_function(Value *(*function)(Context *const, ValueList *const)) {
	Value *val = create_value(FUNCTION);
	val->u.function = create_function(NATIVE_FUNCTION);
	val->u.function->u.native.function = function;
	return val;
}

ExpressionPair *create_expression_pair(Expression *left, Expression *right) {
	#ifdef DEBUG_LANG
	d("create_expression_pair");
	#endif
	ExpressionPair *pair = malloc(sizeof(ExpressionPair));
	pair->left = left;
	pair->right = right;
	return pair;
}

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right) {
	#ifdef DEBUG_LANG
	d("create_binary_expression");
	#endif
	Expression *expr = create_expression(type);
	expr->u.pair = create_expression_pair(left, right);
	return expr;
}

Expression *create_value_expression(Value *value) {
	#ifdef DEBUG_LANG
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
	#ifdef DEBUG_LANG
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
	#ifdef DEBUG_LANG
		d("create_function_call_expression");
	#endif
	Expression *expr = create_expression(FUNCTION_CALL);
	expr->u.function_call = malloc(sizeof(FunctionCall));
	expr->u.function_call->identifier = identifier;
	expr->u.function_call->argument_list = argument_list;
	return expr;
}

ValueList *create_value_list(Value *value) {
	ValueList *vl = malloc(sizeof(ValueList));
	vl->value = value;
	vl->next = NULL;
	return vl;
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
				break;\
			case INTEGER:\
				tmp = create_float_point(\
						(left)->u.float_point op (right)->u.integer\
				);\
				break;\
			}\
			break;\
		case INTEGER:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_float_point(\
						(left)->u.integer op (right)->u.float_point\
				);\
				break;\
			case INTEGER:\
				tmp = create_integer(\
						(left)->u.integer op (right)->u.integer\
				);\
				break;\
			}\
			break;\
		}\
		tmp;\
	})

Value *eval(Context *const context, Expression const *const expression) {
	#ifdef DEBUG_LANG
	d("eval");
	#endif
	#ifdef DEBUG
	if (!expression) {
		d("expression == NULL");
		exit(1);
	}
	#endif
	switch (expression->type) {
	case VALUE:
		{
			#ifdef DEBUG_LANG
				d("VALUE");
			#endif
			if (expression->u.value->type == FUNCTION
					&& expression->u.value->u.function->type == FOREIGN_FUNCTION) {
				expression->u.value->u.function->u.foreign.context = context;
			}
			return expression->u.value;
		}
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		{
			Value const *left;
			Value const *right;
			#ifdef DEBUG_LANG
				d("ADD | SUB | MUL | DIV");
			#endif
			left = eval(context, expression->u.pair->left);
			right = eval(context, expression->u.pair->right);
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
	case ASSIGN:
		{
			char const *name;
			Variable *var;
			#ifdef DEBUG_LANG
				d("ASSIGN");
			#endif
			name = expression->u.assign->identifier;
			var = get_variable(context, name);
			if (!var) {
				var = create_variable(name);
				add_local_variable(context, var);
			}
			var->value = eval(context, expression->u.assign->operand);
			return var->value;
		}
	case IDENTIFIER:
		{
			Variable *var;
			#ifdef DEBUG_LANG
				d("IDENTIFIER");
			#endif
			var = get_variable(context, expression->u.identifier);
			if (var) {
				return var->value;
			} else {
				fprintf(stderr, "fail to eval: no such variable: %s\n", expression->u.identifier);
				exit(1);
			}
		}
	case BLOCK:
		{
			ExpressionList const *el;
			Value *val;
			#ifdef DEBUG_LANG
				char str[80];
				d("BLOCK");
				if (!expression->u.expression_list) {
					d("!expression->u.expression_list");
					exit(1);
				}
			#endif
			for (el = expression->u.expression_list; el; el = el->next) {
				#ifdef DEBUG_LANG
					if (el->expression == NULL) {
						d("el->expression == NULL");
						exit(1);
					}
				#endif
				val = eval(context, el->expression);
				#ifdef DEBUG_LANG
					value2string(str, sizeof(str) - 1, val);
					d("%s", str);
				#endif
			}
			return val;
		}
	case FUNCTION_CALL:
		{
			Variable *var;
			#ifdef DEBUG_LANG
				d("FUNCTION_CALL");
			#endif
			var = get_variable(context, expression->u.function_call->identifier);
			if (var) {
				if (var->value->type == FUNCTION) {
					Function *func;
					func = var->value->u.function;
					Context *fc;
					fc = create_context();
					fc->outer = func->u.foreign.context;
					if (func->type == FOREIGN_FUNCTION) {
						IdentifierList const *pl;
						ExpressionList const *el;
						for (pl = func->u.foreign.parameter_list, el = expression->u.function_call->argument_list;
						     pl && el;
						     pl = pl->next, el = el->next) {
							Variable *var = create_variable(pl->identifier);
							var->value = eval(context, el->expression);
							add_local_variable(fc, var);
						}
						if (pl) {
							fprintf(stderr, "fail to eval: too few parameters: %s\n", var->name);
							exit(EXIT_FAILURE);
						}
						if (el) {
							fprintf(stderr, "fail to eval: too few arguments: %s\n", var->name);
							exit(EXIT_FAILURE);
						}
						return eval(fc, func->u.foreign.expression);
					} else if (func->type == NATIVE_FUNCTION) {
						ValueList *vl, *vh;
						ExpressionList const *el;
						el = expression->u.function_call->argument_list;
						if (el) {
							vh = vl = create_value_list(eval(context, el->expression));
							el = el->next;
						}
						for (; el; el = el->next, vl = vl->next) {
							vl->next = create_value_list(eval(context, el->expression));
						}
						return func->u.native.function(fc, vh);
					} else {
						fprintf(stderr, "fail to eval: bad function type: %d\n", expression->type);
						exit(1);
					}
				}
			}
			fprintf(stderr, "fail to eval: no such function: %s\n", expression->u.function_call->identifier);
			exit(EXIT_FAILURE);
		}
	case OUTER:
		{
			Variable *var;
			#ifdef DEBUG_LANG
				d("OUTER");
			#endif
			var = add_outer_variable(context, expression->u.identifier);
			if (var) {
				return var->value;
			} else {
				fprintf(stderr, "fail to eval: no such outer variable: %s\n", expression->u.identifier);
				exit(EXIT_FAILURE);
			}
		}
	case INNER_ASSIGN:	// TODO
		{
			Variable *var = create_variable(expression->u.assign->identifier);
			add_inner_variable(context, var);
			var->value = eval(context, expression->u.assign->operand);
			return var->value;
		}
	default:
		fprintf(stderr, "fail to eval: bad expression type: %d\n", expression->type);
		exit(1);
	}
}

#undef BINEXP

int value2string(char *string, size_t size, const Value *value) {
	switch (value->type) {
	case INTEGER:
		return snprintf(string, size, "%d", value->u.integer);
	case FLOAT:
		return snprintf(string, size, "%lf", value->u.float_point);
	case FUNCTION:
		{
			int n = 0;
			IdentifierList const *il;
			n += snprintf(string, size, "func(");
			il = value->u.function->u.foreign.parameter_list;
			if (il) {
				strncat(string, il->identifier, size - n);
				n += strlen(il->identifier);
				for (il = il->next; il; il = il->next) {
					strncat(string, ", ", size - n);
					n++;
					strncat(string, il->identifier, size - n);
					n += strlen(il->identifier);
				}
			}
			strncat(string, ")", size - n);
			n++;
			return n;
		}
	case NULL_VALUE:
		return snprintf(string, size, "null");
	default:
		fprintf(stderr, "bad value type: %d\n", value->type);
		exit(1);
	}
}

Variable *create_variable(char const *const name) {
	Variable *var = malloc(sizeof(Variable));
	var->name = name;
	var->value = NULL;
	return var;
}

Context *create_context(void) {
	Context *ctx = malloc(sizeof(Context));
	ctx->variable_list = NULL;
	ctx->outer = NULL;
	return ctx;
}

static Script *compile_script;

void set_compile_script(Script *script) {
	compile_script = script;
}

Script *get_compile_script(void) {
	return compile_script;
}

Script *create_script(void) {
	Script *script = malloc(sizeof(Script));
	script->expression = NULL;
	script->global_context = create_context();
	return script;
}

ExpressionList *create_expression_list(Expression *expression) {
	ExpressionList *el;
	#ifdef DEBUG_LANG
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
	#ifdef DEBUG_LANG
		d("create_identifier_list");
	#endif
	il = malloc(sizeof(IdentifierList));
	il->identifier = identifier;
	il->next = NULL;
	return il;
}

void add_identifier(IdentifierList *identifier_list, char *identifier) {
	IdentifierList *il;
	#ifdef DEBUG_LANG
		d("add_identifier");
	#endif
	for (il = identifier_list; il->next; il = il->next);
	il->next = create_identifier_list(identifier);
}

Expression *create_block_expression(ExpressionList *expression_list) {
	#ifdef DEBUG_LANG
	d("create_block_expression");
	#endif
	Expression *expr;
	expr = create_expression(BLOCK);
	expr->u.expression_list = expression_list;
	return expr;
}

Expression *create_outer_expression(char const *const identifier) {
	#ifdef DEBUG_LANG
		d("create_outer_expression");
	#endif
	Expression *expr = create_expression(OUTER);
	expr->u.identifier = identifier;
	return expr;
}

Expression *create_inner_assign_expression(char const *const identifier, Expression const *const expression) {
	#ifdef DEBUG_LANG
		d("create_inner_expression");
	#endif
	return create_assign_sub(INNER_ASSIGN, identifier, expression);
}

void add_inner_variable(Context *const context, Variable *const variable) {
	#ifdef DEBUG_LANG
		d("add_inner_variable");
	#endif
	VariableList *crt, *vl;
	crt = malloc(sizeof(VariableList));
	crt->variable = variable;
	crt->next = NULL;
	if (context->inner_variable_list) {
		GET_LAST(vl, context->inner_variable_list);
		vl->next = crt;
	} else {
		context->inner_variable_list = crt;
	}
}

/**
 * @return 成功時、Variable のアドレス。失敗時。NULL。
 */
Variable *add_outer_variable(Context *const context, char const *const name) {
	if (context->outer) {
		Variable *var = get_variable(context->outer, name);
		if (var) {
			add_variable(context, var, OUTER_VARIABLE);
			return var;
		}
	}
	return NULL;
}

void add_local_variable(Context *const context, Variable *const variable) {
	add_variable(context, variable, LOCAL_VARIABLE);
}

void add_variable(Context *const context, Variable *const variable, VariableType const type) {
	TypedVariableList *crt, *tvl;
	crt = malloc(sizeof(TypedVariableList));
	crt->variable = variable;
	crt->type = type;
	crt->next = NULL;
	if (context->variable_list) {
		for (tvl = context->variable_list; tvl->next; tvl = tvl->next);
		tvl->next = crt;
	} else {
		context->variable_list = crt;
	}
}

Variable *get_variable(Context const *const context, char const *const name) {// TODO
	#ifdef DEBUG_LANG
		d("get_variable");
	#endif
	Context const *outer;
	FOR (outer, context, outer) {
		VariableList *vl;
		FOR (vl, outer->inner_variable_list, next) {
			if (!strcmp(vl->variable->name, name)) {
				return vl->variable;
			}
		}
	}
	if (context->variable_list) {
		TypedVariableList *tvl;
		for (tvl = context->variable_list; tvl; tvl = tvl->next) {
			if (!strcmp(tvl->variable->name, name)) {
				return tvl->variable;
			}
		}
	}
	return NULL;
}

static void prepare_builtin_function(Context *const context) {
	Variable *var;
	var = create_variable("puts");
	var->value = create_native_function(lang_puts);
	add_inner_variable(context, var);
}

void interpret(Script *script) {
	#ifdef DEBUG_LANG
	d("interpret");
	#endif
	ExpressionList *el;
	char str[80];
	prepare_builtin_function(script->global_context);
	value2string(str, sizeof(str), eval(script->global_context, script->expression));
	printf("%s\n", str);
}

int compile(FILE *input) {
	extern int yyparse(void);
	extern FILE *yyin;

	yyin = input;
	return yyparse();
}

