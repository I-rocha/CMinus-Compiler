#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils.h"
#include "ld.h"

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

listVar* newListVar(){
	listVar* lv;
	lv = (listVar*) malloc(sizeof(listVar));
	lv->list = NULL;
	lv->len = 0;
	lv->fun = NULL;
	return lv;
}


/* Add if not exist */
int addListVar(listVar* lv, char* str, int len){

	if(!lv || !str)
		return 0;
	
	// Already exists
	if(getKeyListVar(lv, str) >= 0)
		return 0;

	lv->list = (varDef*)realloc(lv->list, sizeof(varDef) * (lv->len+1));
	allocateValidator((void**)&lv->list, REALLOC_VALIDATE);

	lv->list[lv->len].var = strdup(str);
	lv->list[lv->len++].len = len;
	return 1;
}

int getKeyListVar(listVar* lv, char* str){
	int pos = 0;
	if(!lv || !lv->list)
		return -1;

	for(int i = 0; i < lv->len; i++){
		if(strcmp(lv->list[i].var, str) == 0)
			return pos;
		pos += (lv->list[i].len > 0) ? lv->list[i].len : 1;
	}
	return -1;
}

int isArray(listVar* lv, char* str){
	if(!lv || !lv->list)
		return -1;	

	for(int i = 0; i < lv->len; i++){
		if(strcmp(lv->list[i].var, str) == 0)
			return (lv->list[i].len > 0) ? 1 : 0;
	}
	
	return -1;
}


int getLenListVar(listVar* lv){
	int len = 0;
	varDef* vd;

	if(!lv || !lv->list)
		return -1;

	// Look for var definition and get len
	for(int i = 0; i < lv->len; i++){
		vd = &lv->list[i];	// var def
		if(vd->len > 0)
			len += vd->len;
		else
			len++;
	}
	return len;
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

void printLDString(listDefinition* l){
	if(!l)
		return;
	printf("List: \n");
	for(int i = 0; i < l->len; i++)
		printf("Line %d -- func %s\n", l->itemStr[i].line, l->itemStr[i].str);
}

void ldPrint(listDefinition* l){
	if(!l)
		return;

	switch(l->type){
	case DEF_ID:
		for(int i = 0; i < l->len; i++){
			printf("(%d, L%d) # ", l->itemId[i].line, l->itemId[i].id);
		}
		printf("\n");
		return;
		break;
	case DEF_STR:
		for(int i = 0; i < l->len; i++){
			printf("%d # L(%s)", l->itemStr[i].line, l->itemStr[i].str);
		}
		printf("\n");
		return;
		break;
	}
	printf("No list definition type (%s)\n", __func__);
	return;
}

void* ldGet(listDefinition* l, void* def){
	definitionByID* itID;
	definitionByStr* itStr;

	if(!l)
		return NULL;

	// Look each item and see if definition exists
	for(int i = 0; i < l->len; i++){
		// Type of union
		if(l->type == DEF_ID){
			itID = &l->itemId[i];		// get item
			if(itID->id == *(int*)def)	// check definition
				return itID;
		}
		// Type of union
		else if(l->type == DEF_STR){
			itStr = &l->itemStr[i];	// get item
			if(strcmp(itStr->str, (char*)def) == 0)	// check definition
				return itStr;
		}
	}
	return NULL;
}

/* Add definition to list if element not exist*/
int ldAdd(listDefinition* l, void* def, int line, int* addr){

	if(!l){
		printf("List not exist (addList)\n");
		exit(0);
		return -1;
	}
	/*
	if(ldGet(l, def))
		return 0;
		*/

	// Check type and alloc new item
	switch(l->type){
	case DEF_ID:
		l->itemId = (definitionByID*)realloc(l->itemId, sizeof(definitionByID) * (l->len + 1));
		allocateValidator((void**)&l->itemId, REALLOC_VALIDATE);
		l->itemId[l->len].id = *(int*)def;
		l->itemId[l->len].line = line;
		l->itemId[l->len].addr = addr;
		l->len++;
		return 1;
		break;
	case DEF_STR:
		l->itemStr = (definitionByStr*)realloc(l->itemStr, sizeof(definitionByStr) * (l->len + 1));
		allocateValidator((void**)&l->itemStr, REALLOC_VALIDATE);
		l->itemStr[l->len].str = strdup((char*)def);
		l->itemStr[l->len].line = line;
		l->itemStr[l->len].addr = addr;
		l->len++;
		return 1;
		break;
	default:
		return -2;
	}
	return 1;
}

/* 1: Removed
 * 0: Definition not exist
 * -1: Error
 */
int ldRm(listDefinition* l, void* def){
	definitionByID* replace_id_t, *lastItemID;
	definitionByStr* replace_str_t, *lastItemStr;
	char* str_ref_d;
	int* addr_ref_d;

	void* toRemove;

	if(!l || l->len == 0)
		return 0;

	toRemove = ldGet(l, def);
	if(!toRemove)
		return 0;

	// Check type, copy last item into replace item and free old allocated types (fields of replaced item)
	switch(l->type){
	case DEF_ID:
		lastItemID = &l->itemId[l->len-1];
		replace_id_t = (definitionByID*)toRemove;
		addr_ref_d = replace_id_t->addr;	// Saving to free later

		// Copy
		replace_id_t->id = lastItemID->id;
		replace_id_t->line = lastItemID->line;
		replace_id_t->addr = lastItemID->addr;

		// Resizing array
		l->itemId = (definitionByID*)realloc(l->itemId, sizeof(definitionByID) * (--l->len));

		if(l->len == 0){
			free(l->itemId);
			l->itemId = NULL;
		}

		free(addr_ref_d);
		break;

	case DEF_STR:
		lastItemStr = &l->itemStr[l->len-1];
		replace_str_t = (definitionByStr*)toRemove;
		str_ref_d = replace_str_t->str;	// Saving to free later

		// Copy
		replace_str_t->str = lastItemStr->str;
		replace_str_t->line = lastItemStr->line;
		replace_str_t->addr = lastItemStr->addr;

		// Resizing array
		l->itemStr = (definitionByStr*)realloc(l->itemStr, sizeof(definitionByStr) * (--l->len));

		if(l->len == 0){
			free(l->itemStr);
			l->itemStr = NULL;
		}

		free(str_ref_d);
		break;

	default:
		return 0;
	}

	return 1;
}

listArg* newListArg(){
	listArg* la;
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

int addListArg(listArg* la, char* str, int isAlias){
	if(!la)
		return 0;

	if(getKeyListArg(la, str) == -1)
		return 0;

	la->list = (argDef*)realloc(la->list, sizeof(argDef) * (la->len+1));
	allocateValidator((void**)&la->list, REALLOC_VALIDATE);

	la->list[la->len].name = strdup(str);
	la->list[la->len++].isAlias = isAlias;
	return 1;
}

int addEmptyListArg(listArg* la, int isAlias){
	if(!la)
		return 0;

	la->list = (argDef*)realloc(la->list, sizeof(argDef) * (la->len+1));
	allocateValidator((void**)&la->list, REALLOC_VALIDATE);

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
	return dict_arg;
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