#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define LPADDING 10

char* lexformat(char* tok, char* lex){
	int sz;
	char hollow[] = " ";

	sz = strlen(tok);
	
	while(sz < LPADDING){
		strcat(tok, hollow);
		sz++;
	}
	strcat(tok, lex);
	
	return strcat(tok, "\n");
}

void freeNull(void** ptr){
	if(!*ptr)
		return;

	free(*ptr);
	*ptr = NULL;
	
	return;
}

void allocateValidator(void** ptr, const char* msg, const char* func){
	char err_txt[100];
	sprintf(err_txt, "%s. (%s)\n", msg, func);

	if(*ptr)
		return;
	printf("%s", err_txt);
	exit(0);
}
/*
listVar* newListVar(){
	listVar* lv;
	lv = (listVar*) malloc(sizeof(listVar));
	lv->list = NULL;
	lv->len = 0;
	lv->fun = NULL;
	return lv;
}
*/

/* Adiciona se nao existe */
/*
int addListVar(listVar* lv, char* str, int len){
	if(!lv)
		return 0;

	if(getKeyListVar(lv, str) == -1)
		return 0;

	lv->list = (varDef*)realloc(lv->list, sizeof(varDef) * (lv->len+1));
	allocateValidator((void**)&lv->list, REALLOC_VALIDATE);

	lv->list[lv->len].var = strdup(str);
	lv->list[lv->len++].len = len;
	return 1;
}
*/
/*
int getKeyListVar(listVar* lv, char* str){
	if(!lv || !lv->list)
		return -1;

	for(int i = 0; i < lv->len; i++){
		if(strcmp(lv->list[i].var, str) == 0)
			return i;
	}
	return -1;
}


int getLenListVar(listVar* lv, char* str){
	varDef* vd;

	if(!lv || !lv->list)
		return -1;

	// Look for var definition and get len
	for(int i = 0; i < lv->len; i++){
		vd = &lv->list[i];	// var def

		if(strcmp(vd->var, str) == 0)
			return vd->len;
	}
	return -1;
}

void freeListVar(listVar* lv){
	if(!lv)
		return;

	if(!lv->list){
		free(lv);
		lv = NULL;
		return;
	}

	for(int i = 0; i < lv->len; i++){
		if(!lv->list[i].var)
			continue;
		free(lv->list[i].var);
	}
	free(lv->list);
	freeNull((void**)&lv);
}
*/
stack* addStack(stack* ptr, char* id){
	stack* new;

	// Allocate new struct
	new = (stack*)malloc(sizeof(stack));
	allocateValidator((void**)&new, MALLOC_VALIDATE);

	new->id = id;

	// Check if ptr already exist and update next
	if(!ptr){
		new->next = NULL;
	}
	else
		new->next = ptr;
	
	ptr = new;

	return ptr;
}

char* popStack(stack** ptr){
	stack* toRemove;
	char* ret;

	if(!ptr || !*ptr)
		return NULL;
	
	ret = (*ptr)->id;

	toRemove = *ptr;
	*ptr = (*ptr)->next;

	freeNull((void**)&toRemove);
	return ret;
}
/*
dictVar* initDictVar(char** str, int nfun){
	dictVar* dict_var;
	listVar* lvar;

	dict_var = (dictVar*) malloc(sizeof(dictVar));
	allocateValidator((void**)&dict_var, MALLOC_VALIDATE);

	dict_var->nfun = nfun;
	dict_var->multiple_lvar = (listVar*)malloc(sizeof(listVar) * nfun);
	allocateValidator((void**)&dict_var, MALLOC_VALIDATE);

	for(int i = 0; i < nfun; i++){
		lvar = &dict_var->multiple_lvar[i];
		lvar->list = NULL;
		lvar->fun = strdup(str[i]);
		lvar->len = 0;
	}
	dict_var->nfun = nfun;
	return dict_var;
}

listVar* getListVar(dictVar* dict_var, char* fun){
	listVar* lvar;
	if(!dict_var)
		return NULL;

	for(int i = 0; i < dict_var->nfun; i++){
		lvar = &dict_var->multiple_lvar[i];
		if(!lvar->fun){
			printf("List of var without function associated (%s)\n", __func__);
			return NULL;
		}
		if(strcmp(lvar->fun, fun) == 0)
			return lvar;
	}
	return NULL;
}

listVar* newListArg(){
	listARg* la;
	la = (listArg*) malloc(sizeof(listArg));
	allocateValidator((void**)&la, MALLOC_VALIDATE);
	la->list = NULL;
	la->len = 0;
	la->fun = NULL;
	return la;
}

int getKeyListArg(listArg* la, char* str){
	if(!la || !la->list)
			return -1;

	for(int i = 0; i < la->len; i++){
		if(strcmp(la->list[i].name, str) == 0)
			return i;
	}
	return -1;	
}

int addListArg(listArg* la, char* str, int len, int isAlias){
	if(!la)
		return 0;

	if(getKeyListVar(la, str) == -1)
		return 0;

	la->list = (argDef*)realloc(la->list, sizeof(argDef) * (la->len+1));
	allocateValidator((void**)&la->list, REALLOC_VALIDATE);

	la->list[la->len].name = strdup(str);
	la->list[la->len++].isAlias = isAlias;
	return 1;
}

dictArg* initDictArg(char** str, int nfun){
	dictArg* dict_arg;
	listArg* larg;

	dict_arg = (dictArg*) malloc(sizeof(dictArg));
	allocateValidator((void**)&dict_arg, MALLOC_VALIDATE);

	dict_arg->nfun = nfun;
	dict_arg->multiple_larg = (listArg*)malloc(sizeof(listArg) * nfun);
	allocateValidator((void**)&dict_arg, MALLOC_VALIDATE);

	for(int i = 0; i < nfun; i++){
		larg = &dict_arg->multiple_larg[i];
		larg->list = NULL;
		larg->fun = strdup(str[i]);
		larg->len = 0;
	}
	dict_arg->nfun = nfun;
	return dict_aeg;
}

listArg* getListArg(dictArg* dict_arg, char* fun){
	listArg* larg;
	if(!dict_arg)
		return NULL;

	for(int i = 0; i < dict_arg->nfun; i++){
		larg = &dict_arg->multiple_larg[i];
		if(!larg->fun){
			printf("List of arg without function associated (%s)\n", __func__);
			return NULL;
		}
		if(strcmp(larg->fun, fun) == 0)
			return larg;
	}
	return NULL;
}
*/
