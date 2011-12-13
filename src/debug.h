#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG
#	define d(...)\
		fprintf(stdout, "%s: %d: ", __FILE__, __LINE__);\
		fprintf(stdout, __VA_ARGS__);\
		fprintf(stdout, "\n")

#	define TEST
#else
#	define d(...)
#endif

#endif

