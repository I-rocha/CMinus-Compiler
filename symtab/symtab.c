#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "../ast.h"
#include "../file_log.h"

// TODO:Change hash type
int hashFunction(char* name){
	if(name == NULL){
		printf(H_ERR_2);
		return 0;
	}
	return (name[0] - 'a' + 1); //INDEX linear
}

int addNo(symbol sym){
	int idx;
	symbol look, aux;

	if(sym == NULL){
		printf(H_ERR_1);
		return -1;
	}

	idx = hashFunction(sym->content.name);
	look = hash[idx];


	aux = (symbol)malloc(sizeof(struct symbol));
	aux->content.type = NULL;
	aux->content.name = NULL;
	aux->content.scope = NULL;
	aux->prox = NULL;

	aux->content.name = strdup(sym->content.name);
	if(sym->content.type != NULL)
		aux->content.type = strdup(sym->content.type);
	if(sym->content.scope != NULL)
		aux->content.scope = strdup(sym->content.scope);
	aux->content.var_func = sym->content.var_func;

	if(look == NULL){
		hash[idx] = aux;
		return 1;
	}
	
	/*LISTA linear*/
	/*TODO Alterar modo de lista para otimizar*/

	while(look->prox != NULL){
		look = look->prox;
	}
	look->prox = aux;
	return 1;
}

symbol getNo(symbol sym){
	int idx;
	symbol look;

	if(sym == NULL){
		printf(H_ERR_1);
		return NULL;
	}

	idx = hashFunction(sym->content.name);
	look = hash[idx];

	while(look != NULL){
		if(isEqual(look, sym))
			return look;
		look = look->prox;
	}

	return NULL;
}

int isNameEqual(symbol symA, symbol symB){
	if(symA == NULL || symB == NULL){
		printf(H_ERR_0);
		return -1;
	}
	else if(symA->content.name == NULL || symB->content.name == NULL){
		printf(H_ERR_2);
		return 0;
	}

	return (strcmp(symA->content.name, symB->content.name) == 0)? 1:0;
}

int isScopeEqual(symbol symA, symbol symB){
	if(symA == NULL || symB == NULL){
		printf(H_ERR_0);
		return -1;
	}
	else if(symA->content.scope == NULL || symB->content.scope == NULL){
		printf(H_ERR_2);
		return 0;
	}

	return (strcmp(symA->content.scope, symB->content.scope) == 0)? 1:0;
}

int isEqual(symbol symA, symbol symB){
	if(isNameEqual(symA, symB) && isNameEqual(symA,symB));
		return 1;

	return 0;
}



int exist(char* name, char* scope){
	int idx;
	symbol look;
	struct symbol aux;

	if(name == NULL) return -1;
	else if(scope == NULL) return -1;

	aux.content.name = strdup(name);
	aux.content.scope = strdup(scope);
	
	idx = hashFunction(name);
	look = hash[idx];

	while(look != NULL){
		if(isScopeEqual(look, &aux) && isNameEqual(look, &aux))
			return 1;
		look = look->prox;
	}

	return 0;
}

char* getType(char* name, char* scope){
	struct symbol aux, *no;
	aux.content.name = strdup(name);
	aux.content.scope = strdup(scope);

	no = getNo(&aux);

	if(aux.content.name != NULL)
		free(aux.content.name);

	if(aux.content.scope != NULL)
		free(aux.content.scope);

	return no->content.type;
}


int printAll(){
	symbol rlook;
	for(int i =0; i < H_MAX; i++){
		rlook = hash[i];

		while(rlook != NULL){
			printContent(rlook);
			rlook = rlook->prox;
		}
	}
	return 1;
}

int printContent(symbol el){
	printf("\n##### SYMBOL #####\n");
	printf("Escopo:\t %s\n", el->content.scope);
	printf("Name:\t %s\n", el->content.name);
	printf("Tipo:\t %s\n", el->content.type);
	printf("##### SYMBOL #####\n");

	return 0;
}


int fprintAll(){
	symbol rlook;
	for(int i =0; i < H_MAX; i++){
		rlook = hash[i];

		while(rlook != NULL){
			fprintContent(rlook);
			rlook = rlook->prox;
		}
	}
	return 1;
}

int fprintContent(symbol el){
	fprintf(sem_log, "\n##### SYMBOL #####\n");
	fprintf(sem_log, "Escopo:\t %s\n", el->content.scope);
	fprintf(sem_log, "Name:\t %s\n", el->content.name);
	fprintf(sem_log, "Tipo:\t %s\n", el->content.type);
	fprintf(sem_log, "##### SYMBOL #####\n");

	return 0;
}
