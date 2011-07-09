#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

int main(int argc, char **argv) {
	Expression *expr = create_assign_expression("tmp", create_value_expression(create_integer(10)));
	printf("maked\n");
	if (ASSIGN == expr->type) {
		printf("name: %s\n", expr->u.assign->variable_name);
		printf("operand: %d\n", expr->u.assign->operand->u.value->u.integer);
	}
	return 0;
}

