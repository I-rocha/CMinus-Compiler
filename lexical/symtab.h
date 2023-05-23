#ifndef SYMTAB_H
#define SYMTAB_H

#define NSYMHASH 1000

#define V_DEFINITION 0
#define V_CALL 1

#include <stdarg.h>
// TOKEN
#include "../ast/ast.h"

typedef struct symEntry{

	/* Symbol attributes */
         Token id;
         char* lexeme; 
         Token type;
	 unsigned short attr;
	 Token* param;
 
	 /* Semantic control */
         unsigned short* call;
         unsigned short* def;		// Line of def
         unsigned short sd_idx, sc_idx;	// Semantic definition index & semantic call index
         struct symTable* env;
  
         // Internal control 
         int null;  
         struct symEntry* prox; 
 } symEntry; 
  
 typedef struct symTable{ 
         char* scope; 
         int len;        // Len of children 
         struct symEntry* table; 
         struct symTable *child, *parent; 
 } symTable;




/*
 * Symbol Table Free Linked List */
int symTFLL(symEntry* no);

/*
 * */
int symTDeepFree(symTable* hash);

/*
 * */
int symTFree(symTable* hash);

/*
 * */
int symTKey(symEntry* item);

/*
 *
 * Creates struct of the item
 * This function allocates data inside the struct but not the struct itself
 * (copy): copy of the struct*/
//symEntry symTNewNo(Token tok, char* lexeme, Token type, int attr);

/*
 * Checks if names are equal
 * 1: equal
 * -1: different
 * */
int symTNameEquals(char* n1, char* n2);

/*
 * Checks if 2 items has same name
 * 1: equal
 * -1: different
 * */
//int symTIsEqual(symEntry* it1, symEntry* it2);

/*
 * Insert new item if not exist on currently environment.
 * This functions allocates only the struct of no
 * 1: Inserted sucessfully
 * 0: Item already exists
 * -1: Error */
//int symTPut(symTable* hash, Token tok, char* lexeme, Token type, int attr, int line);

/*
 * */
void symTPut(symTable* hash, Token tok, char* lexeme, Token type, int attr, int line, int VAR_CATEGORY){

/*
 * Look if definition already exists and returns addres
 * address: Found item
 * NULL: Not found
 * */
//symEntry* symTLook(symTable* hash, Token tok, char* lexeme, Token type);

/*
 * (addr): New environment allocated
 * NULL: Not possible to allocate*/
symTable* symTNewEnv(symTable* hash, char* scope);
unsigned short symTGetNCall(symEntry* entry);
unsigned short symTGetNDef(symEntry* entry);
void symTSetParam(symEntry* entry, int n, ...);

/**/
void symTPrint(symTable* hash, int deep);

/**/
int symTSave(symTable* hash, char* path);

/**/
void symTDeepSave(symTable* hash, int deep, FILE* fd);

symTable* symTExit(symTable* hash);

void symTAddRef(symTable* hash, char* lexeme, int line);
#endif
