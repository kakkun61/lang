#include <stdio.h>
#include "builtin.h"
#include "lang.h"
#include "list-util.h"
#include "create.h"
#include "eval.h"

Value *lang_puts(Context *const context, ValueList *const argument_list) {
	ValueList *vl;
	char str[80];
	FOR (vl, argument_list, next) {
		value2string(str, 80, vl->value);
		printf("%s\n", str);
	}
	return create_value(NULL_VALUE);
}

