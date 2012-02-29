#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "ast.h"
#include "list-util.h"
#include "create.h"
#include "debug.h"

static Value *eval_if(Context *const context, If const *const lang_if);
static Value *eval_foreign_function(Context *const context, Expression const *const expression, char const *name, Function const *const func);
static Value *eval_expression_list(Context *const context, ExpressionList const *expression_list);
static void add_typed_variable_list(Context *const context, TypedVariableList *const variable_list);
static void add_variable(Context *const context, Variable *const variable, VariableType const type);
static Variable *get_variable_from_variable_list(Context const *const context, char const *const name);

static Value *eval_if(Context *const context, If const *const lang_if) {
	d("eval_if");
	Value *val = NULL;
	if (lang_if->condition) {
		val = eval(context, lang_if->condition);
		if (val->type != BOOLEAN) {
			fprintf(stderr, "a type of a condition of \"if\" must be boolean.\n");
			exit(EXIT_FAILURE);
		}
	}
	if (!lang_if->condition || val->u.boolean) { // else 式または条件が真
		return eval(context, lang_if->then);
	} else if (lang_if->elif) {
		return eval_if(context, lang_if->elif);
	}
	fprintf(stderr, "mustn't come here.");
	exit(EXIT_FAILURE);
}

/**
 * @param context    関数の評価された文脈
 * @param expression 関数評価式
 * @param name       関数を参照している変数の名前
 * @param func       関数オブジェクト
 */
static Value *eval_foreign_function(Context *const context, Expression const *const expression, char const *name, Function const *const func) {
	d("eval_foreign_function %s", name);
	Context *fc;
	fc = create_context(FUNCTION_CONTEXT);
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
	if (pl) { // パラメーター数 > アーギュメント数
		fprintf(stderr, "fail to eval: too few parameters: %s\n", name);
		exit(EXIT_FAILURE);
	}
	if (el) { // パラメーター数 < アーギュメント数
		fprintf(stderr, "fail to eval: too few arguments: %s\n", name);
		exit(EXIT_FAILURE);
	}
	return eval(fc, func->u.foreign.expression);
}

static Value *eval_expression_list(Context *const context, ExpressionList const *expression_list) {
	ExpressionList const *el;
	Value *val = NULL;
	#ifdef DEBUG
		char str[80];
	#endif
	#ifdef TEST
		if (!expression_list) {
			d("!expression_list");
			exit(1);
		}
	#endif
	FOR (el, expression_list, next) {
		#ifdef TEST
			if (el->expression == NULL) {
				d("el->expression == NULL");
				exit(1);
			}
		#endif
		val = eval(context, el->expression);
		// TODO return, break, continue の判定
		// TODO Value と制御構文のスーパークラスが必要
		#ifdef DEBUG
			value2string(str, sizeof(str) - 1, val);
			d("=> %s", str);
		#endif
	}
	return val;
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
	d("eval");
	#ifdef TEST
		if (!expression) {
			d("expression == NULL");
			exit(1);
		}
	#endif
	switch (expression->type) {
	case VALUE:
		{
			d("VALUE");
			if (expression->u.value->type == FUNCTION
					&& expression->u.value->u.function->type == FOREIGN_FUNCTION) {
				expression->u.value->u.function->u.foreign.context = context;
			}
			return expression->u.value;
		}
	case ADD:
	case SUB:
	case MUL:
	case MOD:
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
			d("ADD | SUB | MUL | DIV | EQUAL | NOT_EQUAL | GRATER | GRATER_EQUAL | LESS | LESS_EQUAL");
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
			case MOD:
				{
					if (left->type == INTEGER && right->type == INTEGER) {
						return create_integer(left->u.integer % right->u.integer);
					} else {
						fprintf(stderr, "failed to eval: bad value type \"%%\": %d %d\n", (left)->type, (right)->type);
						exit(EXIT_FAILURE);\
					}
				}
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
			default:
				;// TODO エラー処理 不到達
			}
		}
	case MINUS:
		{
			Value *const operand = eval(context, expression->u.expression);
			switch (operand->type) {
			case INTEGER:
				operand->u.integer *=-1;
				return operand;
			case FLOAT:
				operand->u.float_point *=-1;
				return operand;
			default:
				;// TODO エラー処理 不到達
			}
		}
	case ASSIGN:
		{
			char const *name;
			Variable *var;
			d("ASSIGN");
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
			d("IDENTIFIER");
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
			d("BLOCK");
			return eval_expression_list(context, expression->u.expression_list);
		}
	case FUNCTION_CALL:
		{
			Variable *var;
			d("FUNCTION_CALL %s", expression->u.function_call->identifier);
			if (!strcmp(SELF, expression->u.function_call->identifier)) {
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
							Context *const fc = create_context(FUNCTION_CONTEXT);
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
			d("OUTER");
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
			d("IF");
			return eval_if(context, expression->u.lang_if);	
		}
	case FOR:
		{
			d("FOR");
			Value *cond;
			Value *result;
			if (expression->u.lang_for->initialization) {
				eval_expression_list(context, expression->u.lang_for->initialization);
			}
			while (true) {
				if (expression->u.lang_for->condition) {
					cond = eval_expression_list(context, expression->u.lang_for->condition);
					if (cond->type != BOOLEAN) {
						fprintf(stderr, "a value of a condition of \"for\" expression must be boolean.\n");
						exit(EXIT_FAILURE);
					}
					if (!cond->u.boolean) {
						break;
					}
				}
				result = eval(context, expression->u.lang_for->body);
				// TODO break, continue の処理
		
				if (expression->u.lang_for->step) {
					eval_expression_list(context, expression->u.lang_for->step);
				}
			}
			return result;
		}
	default:
		fprintf(stderr, "fail to eval: bad expression type: %d\n", expression->type);
		exit(1);
	}
}

#undef ADDEXP
#undef EQEXP
#undef RELEXP

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

Context *create_context(ContextType type) {
	Context *ctx = malloc(sizeof(Context));
	ctx->type = type;
	ctx->variable_list = NULL;
	ctx->outer = NULL;
	ctx->self = NULL;
	return ctx;
}

void add_inner_variable(Context *const context, Variable *const variable) {
	d("add_inner_variable %s", variable->name);
	#ifdef TEST
	if (!variable) {
		fprintf(stderr, "variable is NULL\n");
		exit(EXIT_FAILURE);
	}
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

static void add_typed_variable_list(Context *const context, TypedVariableList *const variable_list) {
	TypedVariableList *tvl;
	if (context->variable_list) {
		GET_LAST (tvl, context->variable_list, next);
		tvl->next = variable_list;
	} else {
		context->variable_list = variable_list;
	}
}

static void add_variable(Context *const context, Variable *const variable, VariableType const type) {
	TypedVariableList *crt;
	crt = malloc(sizeof(TypedVariableList));
	crt->variable = variable;
	crt->type = type;
	crt->next = NULL;
	add_typed_variable_list(context, crt);
}

/**
 * @return 成功時、Variable へのポインター。失敗時。NULL。
 */
Variable *add_outer_variable(Context *const context, char const *const name) {
	d("add_outer_variable %s", name);
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
	d("add_local_variable %s", variable->name);
	add_variable(context, variable, LOCAL_VARIABLE);
}

Variable *get_variable(Context const *const context, char const *const name) {
	d("get_variable %s", name);
	Context const *outer;
	Variable *var;
	FOR (outer, context, outer) {
		VariableList *vl;
		FOR (vl, outer->inner_variable_list, next) {
			if (!strcmp(vl->variable->name, name)) {
				return vl->variable;
			}
		}
	}
	var = get_variable_from_variable_list(context, name);
	if (var) {
		return var;
	}
	if (context->type == IF_CONTEXT || context->type == FOR_CONTEXT) {
		if (context->outer) {
			return get_variable_from_variable_list(context->outer, name);
		}
	}
	return NULL;
}

static Variable *get_variable_from_variable_list(Context const *const context, char const *const name) {
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

Return *create_return(ReturnType const type, Value const *const value) {
	Return *rtn = malloc(sizeof(Return));
	rtn->type = type;
	rtn->value = value;
	return rtn;
}
