#pragma once

#include <stdio.h>

typedef struct Script_tag Script;

int compile(FILE *input);
void interpret(Script *script);
Script *create_script(void);
Script *get_compile_script(void);
void set_compile_script(Script *script);
