#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../cgen/cgen.h"


typedef struct {
	char item;
} registry;

typedef struct {
	char item;
} glob;

typedef union{
	quad* code;
	glob global;
	registry activation;
} cell;


void envInitGlobal();
void endEnv();


void initFunCode(quad* head);
void test(quad* head);

#endif