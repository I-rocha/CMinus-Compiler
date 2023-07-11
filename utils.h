#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#define MALLOC_VALIDATE "Error, malloc allocation did not work properly", __func__
#define REALLOC_VALIDATE "Error, realloc allocation did not work properly", __func__


/* STRUCTS TO STORE LIST OF ALLOCATED VARS */
/* Var definition 
 * (var): name
 * (len): array-len
 */
/*
typedef struct {
	char* var;
	int len;
}varDef;
*/
/* List of var definition */
/*
typedef struct{
	varDef* list;
	int len;
	char* fun;
}listVar;
*/
/**/
/*
typedef struct{
	listVar* multiple_lvar;
	int nfun;
}dictVar;
*/
/*******************************************/

/*
typedef struct {
	char* name;
	// operation
	int isAlias;
}argDef;
*/
/* List of var definition */
/*
typedef struct{
	argDef* list;
	int len;
	char* fun;
}listArg;
*/
/**/
/*
typedef struct{
	listArg* multiple_larg;
	int nfun;
}dictArg;
*/
/******************************************/

typedef struct Stack{
	char* id;
	struct Stack* next;
}stack;

char* lexformat(char* tok, char* lex);

/* Check if ptr was indeed allocated */
void allocateValidator(void** ptr, const char* msg, const char* func);

/* Free pointer and set null */
void freeNull(void** ptr);


/* List var*/
/*
listVar* newListVar();
int addListVar(listVar* lv, char* str, int len);
int getKeyListVar(listVar* lv, char* str);
void freeListVar(listVar* lv);
int getLenListVar(listVar* lv, char* str);
*/

/* Dict lvar */
/*
dictVar* initDictVar(char** str, int nfun);
listVar* getListVar(dictVar* dict_var, char* fun);
*/
/* List args */
/*
listVar* newListArg();
int addListArg(listArg* la, char* str, int len);
int getKeyListArg(listArg* la, char* str);
*/

/* Dict larg */
/*
dictArg* initDictArg(char** str, int nfun);
listArg* getListArg(dictArg* dict_arg, char* fun);
*/
stack* addStack(stack* ptr, char* id);
char* popStack();

#endif
