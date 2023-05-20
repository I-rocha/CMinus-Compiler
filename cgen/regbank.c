#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "regbank.h"

// First reg and last reg
rprec *freg, *lreg;
rlist *filled;

int putFilled(rprec* reg);

// Inicializa prioridades com valor 0
void regBankInit(){
	rprec* aux;

	freg = (rprec*)malloc(sizeof(rprec));
	freg->idx = 0;
	freg->var = NULL;

	aux = freg;


	for(int i = 1; i < NREG; i++){
		aux->prox = (rprec*)malloc(sizeof(rprec));
		
		aux = aux->prox;

		aux->idx = i;
		aux->var = NULL;

	}
	lreg = aux;
	aux->prox = NULL;
	filled = NULL;
	return;
}

int putFilled(rprec* reg){
	rlist* aux;
	if(!filled){
		filled = (rlist*)malloc(sizeof(rlist));
		filled->prox = NULL;
		filled->reg = reg;
	}
	else{
		aux = (rlist*)malloc(sizeof(rlist));
		aux->prox = filled;
		aux->reg = reg;
		filled = aux;
	}
	return 1;
}

int linkReg(char* var){
	rprec* aux;

	aux = freg;
	putFilled(aux);

	// Move first to last
	lreg->prox = aux;
	freg = freg->prox;
	aux->prox = NULL;
	lreg = lreg->prox;

	// Assign variable linked
	aux->var = var;

	// Return reg idx
	return aux->idx;
}

int getReg(char*var){
	rlist* no;

	no = filled;

	while(no){
		if(!no->reg->var){
			no = no->prox;
			continue;
		}
			
		if(strcmp(no->reg->var, var) == 0)
			return no->reg->idx;
		
		no = no->prox;
	}
	return -1;
}

char* getVar(int reg){
	rlist* no;

	no = filled;

	while(no){
		if((no->reg->idx == reg) && no->reg->var)
			return no->reg->var;

		no = no->prox;
	}
	return NULL;
}

int cleanFilled(){
	rlist *aux;
	
	if(!filled)
		return 1;

	else if(!filled->prox){
		free(filled);
		filled = NULL;
		return 1;
	}

	aux = filled->prox;

	while(aux){
		free(filled);
		filled = aux;
		aux = aux->prox;
	}
	filled = NULL;
	return 1;
}

void printFilled(){
	rlist *no;

	no = filled;

	while(no){
		printf("Reg: %d", no->reg->idx);
		
		if(no->reg->var)
			printf(" Var: %s", no->reg->var);
		
		printf("\n");
		no = no->prox;
	}

	return;
}

/*
int main(){
	init();
	
	linkReg(strdup("v1"));
	linkReg(strdup("v2"));

	printFilled();

	cleanFilled();
	printFilled();
	return 1;

}
*/
