#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../cgen/cgen.h"


typedef struct {
	char item;
} registry;

typedef struct {
	char item;
} glob;

typedef struct union{
	quad* code;
	glob global;
	registry activation;
} cell;




#endif