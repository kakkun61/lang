#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

#ifdef DEBUG
# include "debug.h"
#endif

int main(int argc, char **argv) {
	FILE *input;
	int compile_error;

	if (argc == 1) input = stdin;
	else if (argc == 2) {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			fprintf(stderr, "Fail to open file.\n");
			return 1;
		}
	} else {
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\t%s [source file]\n", argv[0]);
	}

	set_compile_script(create_script());

	compile_error = compile(input);

	if (input != stdin) {
		if (EOF == fclose(input)) {
			fprintf(stderr, "Fail to close \"%s\".\n", argv[1]);
			return 1;
		}
	}

	if (compile_error) {
		return 1;
	}

	interpret(get_compile_script());

	return 0;
}

