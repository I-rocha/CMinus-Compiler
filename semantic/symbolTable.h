#ifndef SYMTAB_H
#define SYMTAB_H
#define VAR 1
#define FUN 0

#include <stdarg.h>
#include <stdbool.h>
// TOKEN
#include "../ast/ast.h"


typedef struct symbolEntry{
    char* name;
    Token type;
    
    /* Semantic control */
    unsigned short* def;		// Line of def

    // Sizes
    unsigned short ndef;
    bool _isFunction;

    // Arrays
    struct {
        int _sizeof;
    };

    // Specific for Function
    struct{
        
        char** params;
        Token* paramsType;
        int nParams;
        unsigned short* call;
        unsigned short ncall;
    };
 } symbolEntry; 
  
 typedef struct symbolTable{ 
         char* scope;
         int size;          // Len of children table
         int len;           // len of env array
         struct symbolEntry* env;
         struct symbolTable *child, *parent;
 } symbolTable;


/**
 * Create new scope reference with scopeName and plug to parent
 */
symbolTable* newScope(char* scopeName, symbolTable* parent);

/**
 * Plug child and parent
 */
void addChild(symbolTable* parent, symbolTable child);

/**
 *  Add new entry to current table
 */
void addEntry(symbolTable* table, symbolEntry entry);

/**
 * Look for definition starting from current scope up to parent's scopes
 */
symbolEntry* lookUp(symbolTable* st, char* name);

/**
 * Look for definition starting from current scope up to parent's scopes
 */
symbolEntry* look(symbolTable* st, char* name);

/**
 * Add another definition line to symbol entry data
 */
void addDefinition(symbolEntry* se, unsigned short defLine);

/**
 * Create a new entry with name, type, lineDefinition and size
 */
symbolEntry newEntry(char* name, Token type, unsigned short lineOfDefinition, int _sizeof);
#endif