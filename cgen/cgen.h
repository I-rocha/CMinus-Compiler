#ifndef CGEN_H
#define CGEN_H

#include "../ast/ast.h"

typedef enum {
	BLANK_C = 0,
	FUN_C,
	ARG_C,
	ARG_ARRAY_C,
	ALLOC_C,
	ALLOC_ARRAY_C,
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
	START_WHILE_C,
	END_WHILE_C,
	START_IF_C,
	END_IF_C,
	START_ELSE_C,
	END_ELSE_C,
	CALL_C,
	NONE_C
	
}CToken;
/*
#define REGT 0
#define LITT 1
*/
typedef struct quad{
	CToken op;
//	int a1, a2, r;
//	int t1, t2, tr;
	char *arg1, *arg2, *result;
	struct quad* next;
}quad;


quad* gen(astNo* astTree);

void printQuad(quad* code);

void printSingle(quad* code);

int saveCI(quad* head, char* path);

char* ctokenStr(CToken tok);


#endif
