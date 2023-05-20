#ifndef CGEN_H
#define CGEN_H

#include "../ast/ast.h"

typedef struct quad{
	char *op, *arg1, *arg2, *result;
	struct quad* next;
}quad;


quad* gen(astNo* astTree);

void printQuad(quad* code);

void printSingle(quad* code);

int saveCI(quad* head, char* path);



#endif
