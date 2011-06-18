#include <stdio.h>
#include <stdlib.h>
	
typedef struct ExpressionS Expression;

typedef enum {
	VALUE,
	ADD,
} ExpressionType;

typedef enum {
	DOUBLE,
} ValueType;

typedef struct {
	ValueType type;
	union {
		double double_value;
	} u;
} Value;

typedef struct ExpressionPairS {
	Expression *left;
	Expression *right;
} ExpressionPair;

typedef struct ExpressionListS {
	Expression *expression;
	struct ExpressionListS *next;
} ExpressionList;

struct ExpressionS {
	ExpressionType type;
	union {
		Value value;
		ExpressionPair pair;
	} u;
};

Expression *create_expression(ExpressionType type) {
	Expression *expr = malloc(sizeof(Expression));
	expr->type = type;
	return expr;
}

Value *create_value(ValueType type) {
	Value *val = malloc(sizeof(Value));
	val->type = type;
	return val;
}

Value *create_double_value(double value) {
	Value *val = create_value(DOUBLE);
	val->u.double_value = value;
	return val;
}

Expression *create_add_expression(Expression *left, Expression *right) {
	Expression *expr = create_expression(ADD);
	expr->u.pair.left = left;
	expr->u.pair.right = right;
	return expr;
}

Expression *create_double_value_expression(double value) {
	Expression *expr = create_expression(VALUE);
	expr->u.value.u.double_value = value;
	return expr;
}

Value *eval(Expression *expression) {
	switch (expression->type) {
	case VALUE:
		return expression->u.value;
	case ADD:
		double tmp = eval(expression->u.pair.left)->u.double_value + eval(expression->u.pair.right)->u.double_value;
		Value *val = create_double_value(tmp);
		return val;
	}
}
		
int main(void) {
/*	extern int yyparse(void);
	extern FILE *yyin;

	yyin = stdin;
	if (yyparse()) {
		fprintf(stderr, "Error\n");
		exit(1);
	}*/
	Expression *expr = create_add_expression(
			create_double_value_expression(1),
			create_double_value_expression(2)
	);
	Value *val = eval(expr);
	printf("%lf\n", val->u.double_value);
}

