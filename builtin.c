#include "builtin.h"
#include "lang.h"
#include <stdio.h>

Value *lang_puts(Context *const context, ValueList *const argument_list) {
	ValueList *vl;
	char str[80];
	FOR (vl, argument_list, next) {
		value2string(str, 80, vl->value);
		printf("%s\n", str);
	}
	return NULL; // TODO null を実装したらそれを返す
}

