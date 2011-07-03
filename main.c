#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

int main(int argc, char **argv) {
	FILE *input;
	if (argc == 1) input = stdin;
	else if (argc == 2) {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			fprintf(stderr, "Fail to open file.\n");
			return 1;
		}
	} else {
		fprintf(stderr, "Bad call.\n");
	}

	set_compile_script(create_script());

	compile(input);

	if (input != stdin) {
		if (EOF == fclose(input)) {
			fprintf(stderr, "Fail to close file.\n");
			return 1;
		}
	}

	interpret(get_compile_script());

	return 0;
}

