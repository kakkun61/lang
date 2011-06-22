#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

int main(void) {
	// float
	Expression *expr = create_binary_expression(
			ADD,
			create_value_expression(create_float_point(1)),
			create_value_expression(create_float_point(2))
	);
	Value *val = eval(expr);
	printf("%lf\n", val->u.float_point);

	expr = create_binary_expression(
			SUB,
			create_value_expression(create_float_point(1)),
			create_value_expression(create_float_point(2))
	);
	val = eval(expr);
	printf("%lf\n", val->u.float_point);

	expr = create_binary_expression(
			MUL,
			create_value_expression(create_float_point(1)),
			create_value_expression(create_float_point(2))
	);
	val = eval(expr);
	printf("%lf\n", val->u.float_point);

	expr = create_binary_expression(
			DIV,
			create_value_expression(create_float_point(1)),
			create_value_expression(create_float_point(2))
	);
	val = eval(expr);
	printf("%lf\n", val->u.float_point);

	// int
	expr = create_binary_expression(
			ADD,
			create_value_expression(create_integer(1)),
			create_value_expression(create_integer(2))
	);
	val = eval(expr);
	printf("%d\n", val->u.integer);

	expr = create_binary_expression(
			SUB,
			create_value_expression(create_integer(1)),
			create_value_expression(create_integer(2))
	);
	val = eval(expr);
	printf("%d\n", val->u.integer);

	expr = create_binary_expression(
			MUL,
			create_value_expression(create_integer(1)),
			create_value_expression(create_integer(2))
	);
	val = eval(expr);
	printf("%d\n", val->u.integer);

	expr = create_binary_expression(
			DIV,
			create_value_expression(create_integer(1)),
			create_value_expression(create_integer(2))
	);
	val = eval(expr);
	printf("%d\n", val->u.integer);

	return 0;
}

