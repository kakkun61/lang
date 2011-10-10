#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lang.h"

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

int value2string(char *string, size_t size, const Value *value);

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

Value *create_function(IdentifierList *parameter_list, Expression *expression) {
	Value *val = create_value(FUNCTION);
	#ifdef DEBUG_LANG
		d("create_function");
	#endif
	#ifdef TEST
		if (expression->type != BLOCK) {
			d("interpreter bug: expression->type != BLOCK");
			exit(1);
		}
	#endif
	val->u.function = malloc(sizeof(Function));
	val->u.function->type = FOREIGN_FUNCTION;
	val->u.function->u.foreign.parameter_list = parameter_list;
	val->u.function->u.foreign.expression_list = expression->u.expression_list;
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

Expression *create_assign_expression(char *variable_name, Expression *operand) {
	#ifdef DEBUG_LANG
	d("create_assign_expression");
	#endif
	Expression *expr = create_expression(ASSIGN);
	expr->u.assign = malloc(sizeof(Assign));
	expr->u.assign->variable_name = variable_name;
	expr->u.assign->operand = operand;
	return expr;
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

Value *eval(Context *context, Expression *expression) {
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
		return expression->u.value;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		{
			Value *left = eval(context, expression->u.pair->left);
			Value *right = eval(context, expression->u.pair->right);
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
			char *name;
			Variable *var;
			name = expression->u.assign->variable_name;
			var = get_variable(context, name);
			if (!var) {
				var = malloc(sizeof(Variable));
				var->name = name;
				add_variable(context, var);
			}
			var->value = eval(context, expression->u.assign->operand);
			return var->value;
		}
	case IDENTIFIER:
		return get_variable(context, expression->u.identifier)->value;
	case BLOCK:
		{
			ExpressionList *el;
			Value *val;
			#ifdef DEBUG_LANG
				char str[80];
				d("eval BLOCK");
				if (!expression->u.expression_list) { // => true
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
			/*Value *val;
			Function func =*/	
			#ifdef DEBUG_LANG
				d("FUNCTION_CALL");
			#endif
			/*return val;*/
			// TODO
			return NULL;
		}
	case OUTER:
		{
			if (EXIT_SUCCESS == add_outer_variable(context, expression->u.identifier)) {
				return get_variable(context, expression->u.identifier)->value;
			} else {
				fprintf(stderr, "fail to eval: no such outer variable: %s\n", expression->u.identifier);
				exit(EXIT_FAILURE);
			}
		}
	default:
		fprintf(stderr, "fail to eval: bad expression type: %d\n", expression->type);
		exit(1);
	}
}

#undef BINEXP

/**
 * size に終端文字は含まない。
 */
int value2string(char *string, size_t size, const Value *value) {
	switch (value->type) {
	case INTEGER:
		return snprintf(string, size, "%d", value->u.integer);
	case FLOAT:
		return snprintf(string, size, "%lf", value->u.float_point);
	case FUNCTION:
		{
			int n = 0;
			IdentifierList *il;
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
	default:
		fprintf(stderr, "bad value type: %d\n", value->type);
		exit(1);
	}
}

Context *create_context(void) {
	Context *ctx = malloc(sizeof(Context));
	ctx->variable_list = NULL;
	ctx->outer_variable_list = NULL;
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

Expression *create_outer_expression(char const *identifier) {
	#ifdef DEBUG_LANG
		d("create_outer_expression");
	#endif
	Expression *expr = create_expression(OUTER);
	expr->u.identifier = identifier;
	return expr;
}

/**
 * @return 成功したとき、EXIT_SUCCESS (stdlib.h)
 */
int add_outer_variable(Context *const context, char const *const name) {
	if (context->outer) {
		Variable *var = get_variable(context->outer, name);
		if (var) {
			OuterVariableList *crt = malloc(sizeof(OuterVariableList));
			crt->variable = var;
			crt->next = NULL;
			if (context->outer_variable_list) {
				OuterVariableList *ovl;
				GET_LAST(ovl, context->outer_variable_list);
				ovl->next = crt;
			} else {
				context->outer_variable_list = crt;
			}
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

void add_variable(Context *const context, Variable *const variable) {
	VariableList *crt, *vl;
	crt = malloc(sizeof(VariableList));
	crt->variable = variable;
	crt->next = NULL;
	if (context->variable_list) {
		for (vl = context->variable_list; vl->next; vl = vl->next);
		vl->next = crt;
	} else {
		context->variable_list = crt;
	}
}

Variable *get_variable(Context const *const context, char const *const name) {
	if (context->outer_variable_list) {
		OuterVariableList *ovl;
		for (ovl = context->outer_variable_list; ovl; ovl = ovl->next) {
			if (!strcmp(ovl->variable->name, name)) {
				return ovl->variable;
			}
		}
	} else if (context->variable_list) {
		VariableList *vl;
		for (vl = context->variable_list; vl; vl = vl->next) {
			if (!strcmp(vl->variable->name, name)) {
				return vl->variable;
			}
		}
	}
	return NULL;
}

void interpret(Script *script) {
	#ifdef DEBUG_LANG
	d("interpret");
	#endif
	ExpressionList *el;
	char str[80];
	value2string(str, sizeof(str), eval(script->global_context, script->expression));
	printf("%s\n", str);
}

int compile(FILE *input) {
	extern int yyparse(void);
	extern FILE *yyin;

	yyin = input;
	return yyparse();
}

