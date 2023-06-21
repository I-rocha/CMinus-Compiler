#ifndef CGEN_H
#define CGEN_H

#include "../ast/ast.h"

typedef enum {
	BLANK_C = 0,
	FUN_C,
	ARG_C,
	ALLOC_C,
	BEGINCODE_C,
	HALT_C,
	ENDCODE_C,
	END_C,
	LOAD_C,
	IFF_C,
	LABEL_C,
	GOTO_C,
	ADD_C,
	SUB_C,
	MULT_C,
	DIV_C,
	LE_C,
	LESS_C,
	GRAND_C,
	GE_C,
	EQ_C,
	DIFF_C,
	ASSIGN_C,
	STORE_C,
	PARAM_C,
	RETURN_C,
	CALL_C,
	NONE_C
	
}CToken;


typedef struct quad{
	CToken op;
	char *arg1, *arg2, *result;
	struct quad* next;
}quad;


quad* gen(astNo* astTree);

void printQuad(quad* code);

void printSingle(quad* code);

int saveCI(quad* head, char* path);

char* ctokenStr(CToken tok);


#endif
