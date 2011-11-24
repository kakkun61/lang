#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "ast.h"
#include "list-util.h"
#include "create.h"

#ifdef DEBUG
#	include "debug.h"
#endif

static Value *eval_foreign_function(Context *const context, Expression const *const expression, char const *name, Function const *const func) {
	#ifdef DEBUG_EVAL
		d("eval_foreign_function %s", name);
	#endif
	Context *fc;
	fc = create_context();
	fc->outer = func->u.foreign.context;
	fc->self = func;
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
		fprintf(stderr, "fail to eval: too few parameters: %s\n", name);
		exit(EXIT_FAILURE);
	}
	if (el) {
		fprintf(stderr, "fail to eval: too few arguments: %s\n", name);
		exit(EXIT_FAILURE);
	}
	return eval(fc, func->u.foreign.expression);
}

#define ADDEXP(op, left, right)\
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
			default:\
				fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
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
			default:\
				fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
			}\
			break;\
		default:\
			fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
			exit(EXIT_FAILURE);\
		}\
		tmp;\
	})

#define EQEXP(op, left, right)\
	({\
		Value *tmp;\
		switch ((left)->type) {\
		case FLOAT:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_boolean(\
						(left)->u.float_point op (right)->u.float_point\
				);\
				break;\
			case INTEGER:\
				tmp = create_boolean(\
						(left)->u.float_point op (right)->u.integer\
				);\
				break;\
			default:\
				fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
			}\
			break;\
		case INTEGER:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_boolean(\
						(left)->u.integer op (right)->u.float_point\
				);\
				break;\
			case INTEGER:\
				tmp = create_boolean(\
						(left)->u.integer op (right)->u.integer\
				);\
				break;\
			default:\
				fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
			}\
			break;\
		case BOOLEAN:\
			switch ((right)->type) {\
			case BOOLEAN:\
				tmp = create_boolean(\
						(left)->u.boolean op (right)->u.boolean\
				);\
				break;\
			default:\
				fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
			}\
			break;\
		default:\
			fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
			exit(EXIT_FAILURE);\
		}\
		tmp;\
	})

#define RELEXP(op, left, right)\
	({\
		Value *tmp;\
		switch ((left)->type) {\
		case FLOAT:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_boolean(\
						(left)->u.float_point op (right)->u.float_point\
				);\
				break;\
			case INTEGER:\
				tmp = create_boolean(\
						(left)->u.float_point op (right)->u.integer\
				);\
				break;\
			default:\
				fprintf(stderr, "failed to eval: bad value type \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
			}\
			break;\
		case INTEGER:\
			switch ((right)->type) {\
			case FLOAT:\
				tmp = create_boolean(\
						(left)->u.integer op (right)->u.float_point\
				);\
				break;\
			case INTEGER:\
				tmp = create_boolean(\
						(left)->u.integer op (right)->u.integer\
				);\
				break;\
			default:\
				fprintf(stderr, "failed to eval: bad value type \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
				exit(EXIT_FAILURE);\
			}\
			break;\
		default:\
			fprintf(stderr, "failed to eval: bad value type for \"%s\": %d %d\n", #op, (left)->type, (right)->type);\
			exit(EXIT_FAILURE);\
		}\
		tmp;\
	})

Value *eval(Context *const context, Expression const *const expression) {
	#ifdef DEBUG_EVAL
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
			#ifdef DEBUG_EVAL
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
	case EQUAL:
	case NOT_EQUAL:
	case GRATER:
	case GRATER_EQUAL:
	case LESS:
	case LESS_EQUAL:
		{
			Value const *left;
			Value const *right;
			#ifdef DEBUG_EVAL
				d("ADD | SUB | MUL | DIV");
			#endif
			left = eval(context, expression->u.pair->left);
			right = eval(context, expression->u.pair->right);
			switch (expression->type) {
			case ADD:
				return ADDEXP(+, left, right);
			case SUB:
				return ADDEXP(-, left, right);
			case MUL:
				return ADDEXP(*, left, right);
			case DIV:
				return ADDEXP(/, left, right);
			case EQUAL:
				return EQEXP(==, left, right);
			case NOT_EQUAL:
				return EQEXP(!=, left, right);
			case GRATER:
				return RELEXP(>,left, right);
			case GRATER_EQUAL:
				return RELEXP(>=,left, right);
			case LESS:
				return RELEXP(<,left, right);
			case LESS_EQUAL:
				return RELEXP(<=,left, right);
			}
		}
	case ASSIGN:
		{
			char const *name;
			Variable *var;
			#ifdef DEBUG_EVAL
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
			#ifdef DEBUG_EVAL
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
			#ifdef DEBUG_EVAL
				char str[80];
				d("BLOCK");
				if (!expression->u.expression_list) {
					d("!expression->u.expression_list");
					exit(1);
				}
			#endif
			for (el = expression->u.expression_list; el; el = el->next) {
				#ifdef DEBUG_EVAL
					if (el->expression == NULL) {
						d("el->expression == NULL");
						exit(1);
					}
				#endif
				val = eval(context, el->expression);
				#ifdef DEBUG_EVAL
					value2string(str, sizeof(str) - 1, val);
					d("%s", str);
				#endif
			}
			return val;
		}
	case FUNCTION_CALL:
		{
			Variable *var;
			#ifdef DEBUG_EVAL
				d("FUNCTION_CALL %s", expression->u.function_call->identifier);
			#endif
			if (!strcmp(SELF, expression->u.function_call->identifier)) {
				d("%s %s", SELF, expression->u.function_call->identifier);
				return eval_foreign_function(context, expression, SELF, context->self);
			} else {
				var = get_variable(context, expression->u.function_call->identifier);
				if (var) {
					if (var->value->type == FUNCTION) {
						Function *func;
						func = var->value->u.function;
						if (func->type == FOREIGN_FUNCTION) {
							return eval_foreign_function(context, expression, var->name, func);
						} else if (func->type == NATIVE_FUNCTION) {
							Context *const fc = create_context();
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
			}
			fprintf(stderr, "fail to eval: no such function: %s\n", expression->u.function_call->identifier);
			exit(EXIT_FAILURE);
		}
	case OUTER:
		{
			Variable *var;
			#ifdef DEBUG_EVAL
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
	case INNER_ASSIGN:
		{
			Variable *var = create_variable(expression->u.assign->identifier);
			add_inner_variable(context, var);
			var->value = eval(context, expression->u.assign->operand);
			return var->value;
		}
	case IF:
		{
			#ifdef DEBUG_EVAL
				d("IF");
			#endif
			return eval_if(context, expression->u.lang_if);	
		}
	default:
		fprintf(stderr, "fail to eval: bad expression type: %d\n", expression->type);
		exit(1);
	}
}

#undef ADDEXP
#undef EQEXP
#undef RELEXP

Value *eval_if(Context *const context, If const *const lang_if) {
	#ifdef DEBUG_EVAL
		d("eval_if");
	#endif
	Value *val = NULL;
	if (lang_if->condition) {
		val = eval(context, lang_if->condition);
		if (val->type != BOOLEAN) {
			fprintf(stderr, "a type of a condition of \"if\" must be boolean.\n");
			exit(EXIT_FAILURE);
		}
	}
	if (!lang_if->condition || val->u.boolean) {
		return eval(context, lang_if->then);
	} else if (lang_if->elif) {
		return eval_if(context, lang_if->elif);
	}
	fprintf(stderr, "mustn't come here.");
	exit(EXIT_FAILURE);
}

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
	case BOOLEAN:
		{
			switch (value->u.boolean) {
			case true:
				return snprintf(string, size, "true");
			case false:
				return snprintf(string, size, "false");
			}
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
	ctx->self = NULL;
	return ctx;
}

void add_inner_variable(Context *const context, Variable *const variable) {
	#ifdef DEBUG_EVAL
		d("add_inner_variable");
	#endif
	VariableList *crt, *vl;
	crt = malloc(sizeof(VariableList));
	crt->variable = variable;
	crt->next = NULL;
	if (context->inner_variable_list) {
		GET_LAST(vl, context->inner_variable_list, next);
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

Variable *get_variable(Context const *const context, char const *const name) {
	#ifdef DEBUG_EVAL
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

ValueList *create_value_list(Value *value) {
	ValueList *vl = malloc(sizeof(ValueList));
	vl->value = value;
	vl->next = NULL;
	return vl;
}

Value *create_native_function(Value *(*function)(Context *const, ValueList *const)) {
	Value *val = create_value(FUNCTION);
	val->u.function = create_function(NATIVE_FUNCTION);
	val->u.function->u.native.function = function;
	return val;
}
