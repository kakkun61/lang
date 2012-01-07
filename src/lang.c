#include <stdio.h>
#include "lang.h"
#include "eval.h"
#include "builtin.h"
#include "create.h"
#include "debug.h"

struct Script_tag {
	Expression *expression;
	Context *global_context;
};

Script *compile_script;

void set_compile_script(Script *script) {
	compile_script = script;
}

Script *get_compile_script(void) {
	return compile_script;
}

Script *create_script(void) {
	Script *script = malloc(sizeof(Script));
	script->expression = NULL;
	script->global_context = create_context(FUNCTION_CONTEXT);
	return script;
}

static void prepare_builtin_function(Context *const context) {
	Variable *var;
	var = create_variable(create_identifier("puts"));
	var->value = create_native_function(lang_puts);
	add_inner_variable(context, var);
}

void interpret(Script *script) {
	d("interpret");
	prepare_builtin_function(script->global_context);
	if (script->expression) {
		eval(script->global_context, script->expression);
	}
}

int compile(FILE *input) {
	extern int yyparse(void);
	extern FILE *yyin;

	yyin = input;
	return yyparse();
}

void set_expression(Script *const script, Expression *const expression) {
	script->expression = expression;
}
