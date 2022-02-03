#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

// TODO: Adjust code to struct with type

/*
int main(int argc, char** argv){
	//for(int i = 0; i < H_MAX; i++){
	//	head[i] = NULL;
	//}
	struct symbol a, b, c, *result;

	a.ival = 20;
	a.escopo = strdup("main");
	a.name = strdup("a");
	a.prox = NULL;

	b.ival = 33;
	b.escopo = strdup("ain");
	b.name = strdup("a");
	b.prox = NULL;

	c.ival = 85;
	c.escopo = strdup("get");
	c.name = strdup("z84");
	c.prox = NULL;


	add(&a);
	add(&b);
	add(&c);

	printAll();

	return 1;
}
*/
int isEqual(symbol a, symbol b){

	if(a == NULL || b==NULL || a->name == NULL || b->name == NULL || a->escopo == NULL || b->escopo == NULL){
		printf(H_ERR_0);
		return -1;
	}

	else if((strcmp(a->name,b->name) == 0) && (strcmp(a->escopo, b->escopo) == 0))
		return 1;
	else
		return 0;
	return 0;
}


int hashFunction(char* name){
	if(name == NULL)printf("Name null\n");
	return (name[0] - 'a' + 1); //INDEX linear
}

int add(char* scope, char* type, char* name, int val){
	struct symbol aux;

	aux.escopo = strdup(scope);
	aux.type = strdup(type);	
	aux.name = strdup(name);
	aux.ival = val;
	aux.prox = NULL;

	addNo(&aux);
	if(aux.escopo != NULL ) free(aux.escopo);
	if(aux.type != NULL ) free(aux.type);
	if(aux.name != NULL ) free(aux.name);

	return 1;
}

int addNo(symbol no){
	int idx;
	symbol look, aux;

	if(no == NULL){
		printf(H_ERR_1);
		return -1;
	}

	idx = hashFunction(no->name);
	look = head[idx];

	aux = (symbol)malloc(sizeof(symbol*));

	aux->type = strdup(no->type);
	aux->name = strdup(no->name);
	aux->escopo = strdup(no->escopo);
	aux->ival = no->ival;
	aux->prox = NULL;

	if(look == NULL){
		head[idx] = aux;
		return 1;
	}

	/*LISTA linear*/
	/*TODO Alterar modo de lista para otimizar*/
	while(look->prox != NULL){
		if(isEqual(look,no)){
			return 0;
		}
		look = look->prox;
	}

	look->prox = aux;
	return 1;
}

symbol get(char* scope, char* name){
	symbol target;
	struct symbol no;

	no.escopo = strdup(scope);
	no.name = strdup(name);
	target = getNo(&no);

	if(no.escopo != NULL)free(no.escopo);
	if(no.name != NULL)free(no.name);

	return target;
}

symbol getNo(symbol no){
	int idx;
	symbol look;

	if(no == NULL){
		printf(H_ERR_1);
		return NULL;
	}

	idx = hashFunction(no->name);
	look = head[idx];

	while(look != NULL){
		if(isEqual(look, no) == 1)
			return look;
		look = look->prox;
	}

	return NULL;
}

int printAll(){
	symbol rlook;
	for(int i =0; i < H_MAX; i++){
		rlook = head[i];

		while(rlook != NULL){
			printContent(rlook);
			rlook = rlook->prox;
		}
	}
	return 1;
}

int printContent(symbol el){
	printf("\n##### VAR #####\n");
	printf("Escopo:\t %s\n", el->escopo);
	printf("Name:\t %s\n", el->name);
	printf("Ival:\t %d\n", el->ival);
	printf("\n##### VAR #####\n");

	return 0;
}
