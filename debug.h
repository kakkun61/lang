#ifndef DEBUG_H
#define DEBUG_H

#define d(...)\
	fprintf(stdout, "%s: %d: ", __FILE__, __LINE__);\
	fprintf(stdout, __VA_ARGS__);\
	fprintf(stdout, "\n")

#endif

