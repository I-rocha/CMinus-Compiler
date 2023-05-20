#ifndef SYMTAB_H
#define SYMTAB_H

#define NSYMHASH 1000


typedef struct symEntry{
	char* tok;
	char* declaration;
	char* attrID;
	char* attrINT;
	char* type;
	int null;
	struct symEntry* prox;
} symEntry;

typedef struct symTable{
	int len;	// Len of children
	struct symEntry* table;
	struct symTable *child, *parent;
} symTable;

/*
 * */
symTable* symTInit();

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
symEntry symTNewNo(char* tok, char* declaration, char* attrID, char* attrINT, char* type);

/*
 * Checks if 2 items are equal
 * 1: equal
 * -1: different
 * */
int symTIsEqual(symEntry* it1, symEntry* it2);

/*
 * Insert new item if not exist on currently environment.
 * This functions allocates only the struct of no
 * 1: Inserted sucessfully
 * 0: Item already exists
 * -1: Error */
int symTPut(symTable* hash, char* tok, char* declaration, char* attrID, char* attrINT, char* type);

/*
 * Look if definition already exists and returns addres
 * address: Found item
 * NULL: Not found
 * */
symEntry* symTLook(symTable* hash, char* tok, char* declaration, char* attrID, char* attrINT);

/*
 * (addr): New environment allocated
 * NULL: Not possible to allocate*/
symTable* symTNewEnv(symTable* hash);

/**/
void symTPrint(symTable* hash, int deep);

/**/
int symTSave(symTable* hash, char* path);

/**/
void symTDeepSave(symTable* hash, int deep, FILE* fd);

#endif
