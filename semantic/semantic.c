#include <stdlib.h>
#include <stdio.h>

#include "semantic.h"
#include "../lexical/symtab.h"

#define MAINF "main"
#define INPUTF "input"
#define OUTPUTF "output"

extern symTable* headEnv;

int checkMain(){
	if(symTLook(headEnv, "ID", "Function", MAINF, NULL))
		return 1;

	printf("Erro Semantico - Funcao main não definida\n");
	return 0;
}

void addIO(){
	symTPut(headEnv, "ID", "Function", INPUTF, NULL, "INT");
	symTPut(headEnv, "ID", "Function", OUTPUTF, NULL, "INT");
}
