#ifndef LD_H
#define LD_H

#define DEF_ID 0 
#define DEF_STR 1 
#include "mnemonic.h"

/************************************************************/
/*	STRUCTS FOR RETRO REPLACEMENT OF ADDRESSES */
/*	List of definitions/calls of labels/functions address */

/* Definitions for labels (defined by it's label number)
 * (id): label number
 * (line): position in-memmory
 **/
typedef struct{
	int id;
	int line;
	int* addr;
}definitionByID;

/* Definition for variables (defined by it's name)
 * (str): Function name
 * (line): position in-memmory
 */
typedef struct{
	char* str;	// var name
	int line;
	int* addr;
}definitionByStr;

typedef struct ListDefinition{
	union{
		definitionByID *itemId;
		definitionByStr *itemStr;
	};
	int len;
	int type;
}listDefinition;
/************************************************************/

/* STRUCTS TO STORE LIST OF ALLOCATED VARS */
/* Var definition 
 * (var): name
 * (len): array-len
 */
typedef struct {
	char* var;
	int len;
}varDef;

/* List of var definition */
typedef struct{
	varDef* list;
	int len;
	char* fun;
}listVar;

/**/
typedef struct{
	listVar* multiple_lvar;
	int nfun;
}dictVar;
/*******************************************/

// STRUCTS TO STORE LIST OF ALLOCATED ARGS
typedef struct {
	char* name;
	memmory* instr_block;
	int isAlias;
}argDef;

typedef struct{
	argDef* list;
	int len;
	char* fun;
}listArg;

typedef struct{
	listArg* multiple_larg;
	int nfun;
}dictArg;
/*******************************************/

/* List var*/
listVar* newListVar();
int addListVar(listVar* lv, char* str, int len);
int getKeyListVar(listVar* lv, char* str);
int isArray(listVar* lv, char* str);
void freeListVar(listVar* lv);
int getLenListVar(listVar* lv);

/* Dict lvar */
dictVar* initDictVar(char** str, int nfun);
listVar* getListVar(dictVar* dict_var, char* fun);

/* Lists definition */
void* ldGet(listDefinition* l, void* def);
int ldAdd(listDefinition* l, void* def, int line, int* addr);
int ldRm(listDefinition* l, void* def);
void ldPrint(listDefinition* l);

/* List arg */
listArg* newListArg();
int addListArg(listArg* la, char* str, int isAlias);
int addEmptyListArg(listArg* la, int isAlias);
int getKeyListArg(listArg* la, char* str);

/* Dict larg */
dictArg* initDictArg(char** str, int nfun);
listArg* getListArg(dictArg* dict_arg, char* fun);

#endif