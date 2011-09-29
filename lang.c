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

Value *eval(Expression *expression) {
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
			Value *left = eval(expression->u.pair->left);
			Value *right = eval(expression->u.pair->right);
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
			var = get_variable(name);
			if (!var) {
				var = malloc(sizeof(Variable));
				var->name = name;
				add_variable(var);
			}
			var->value = eval(expression->u.assign->operand);
			return var->value;
		}
	case IDENTIFIER:
		return get_variable(expression->u.identifier)->value;
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
				val = eval(el->expression);
				#ifdef DEBUG_LANG
					value2string(str, sizeof(str), val);
					d("%s", str);
				#endif
			}
			return val;
		}
	default:
		fprintf(stderr, "bad expression type\n");
		exit(1);
	}
}

#undef BINEXP

int value2string(char *string, size_t size, const Value *value) { // SEGV
	switch (value->type) {
	case INTEGER:
		return snprintf(string, size, "%d", value->u.integer);
		break;
	case FLOAT:
		return snprintf(string, size, "%lf", value->u.float_point);
		break;
	default:
		fprintf(stderr, "bad value type\n");
		exit(1);
	}
}

static Script *compile_script;

void set_compile_script(Script *script) {
	compile_script = script;
}

Script *get_compile_script() {
	return compile_script;
}

Script *create_script() {
	Script *script = malloc(sizeof(Script));
	script->expression = NULL;
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

void add_variable(Variable *variable) {
	VariableList *crt, *vl;
	crt = malloc(sizeof(VariableList));
	crt->variable = variable;
	crt->next = NULL;
	if (compile_script->variable_list) {
		for (vl = compile_script->variable_list; vl->next; vl = vl->next);
		vl->next = crt;
	} else {
		compile_script->variable_list = crt;
	}
}

Variable *get_variable(char *name) {
	VariableList *vl;
	if (compile_script->variable_list) {
		for (vl = compile_script->variable_list; vl; vl = vl->next) {
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
	value2string(str, sizeof(str), eval(script->expression));
	printf("%s\n", str);
}

int compile(FILE *input) {
	extern int yyparse(void);
	extern FILE *yyin;

	yyin = input;
	return yyparse();
}

