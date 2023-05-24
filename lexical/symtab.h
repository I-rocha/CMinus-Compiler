#ifndef SYMTAB_H
#define SYMTAB_H

#define NSYMHASH 1000

#define _DECLARATION 0
#define _CALL 1

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

	 // Sizes
	 unsigned short ndef;
	 unsigned short ncall;
	 unsigned short nparam;

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
 * Init basic struct
 * Init table with first values
 * (table*) - Entire table allocated
 * */
symTable* symTInit();

/* 
 * Calculate Key of symbol table 
 * (key) - position of item in table
 * */
int symTKey(symEntry* item);

/*
 * Checks if names are equal
 * 1: equal
 * -1: different
 * */
int symTNameEquals(char* n1, char* n2);

/*
 * Put symbol in table on local environment. If symbol already exists, nothing happened
 * (addr) - address to entry on table
 * */
symEntry* putLocal(symTable* hash, Token tok, char* lexeme, Token type, int attr, unsigned short line);

/*
 * Put symbol in table on global environment. If symbol already exists, nothing happened
 * (addr) - address to entry on table
 * */
symEntry* putGlobal(symTable* hash, Token tok, char* lexeme, Token type, int attr, unsigned short line);

/*
 * Calls putGlobal or putLocal based on ID_CATEGORY. Also updates field call or def based on ID_CATEGORY 
 * (entry*) - address of entry target stored in table
 * */
symEntry* symTPut(symTable* hash, Token tok, char* lexeme, Token type, int attr, unsigned short line, int ID_CATEGORY);

/*
 * Look in table for entry with name lookup
 * (symEntry*) - ptr to entry
 * NULL - Not found
 * */
symEntry* symTLook(symTable* hash, char* lookup);

/* Add new reference-line of def*/
void updateDef(symEntry* entry, unsigned short line);

/* Add new reference-line of call*/
void updateCall(symEntry* entry, unsigned short line);

/* Get quantity of line-references call */
unsigned short symTGetNCall(symEntry* entry);

/* Get quantity of line-references defines */
unsigned short symTGetNDef(symEntry* entry);

/* Create new env child */
symTable* symTNewEnv(symTable* hash, char* scope);

/* Update field param */
void symTSetParam(symEntry* entry, int n, ...);

/* Add single param */
void symTAddParam(symEntry* entry, Token param);

/* Get number of params*/
int symTGetNParam(symEntry* entry);

/* Print detailed info of entry */
void printSingleDetail(symEntry* entry);

/* Print detailed info of entire hash */
void symTPrint(symTable* hash, int deep);

/* Save table in new file named by path */
int symTSave(symTable* hash, char* path);

/* Save single info in oppened file*/
void saveSingleInfo(symEntry* entry, FILE* fd);

/* Save info in oppened file*/
void saveInfo(symTable* hash, int deep, FILE *fd);

/* 
 * Exit last environment
 * (env) - return parent env */
symTable* symTExit(symTable* hash);

#endif
