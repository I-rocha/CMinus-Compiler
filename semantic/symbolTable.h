#ifndef SYMTAB_H
#define SYMTAB_H
#define VAR 1
#define FUN 0

#include <stdarg.h>
#include <stdbool.h>
// TOKEN
#include "../ast/ast.h"


typedef union symbolEntry{

    // Variable
    struct {
        char* name;
        Token type;
        
        /* Semantic control */
        unsigned short* def;		// Line of def

        // Sizes
        unsigned short ndef;
        struct symTable* env;
        bool _type;
    };

    // Function
    struct{
        char* name;
        Token type;
        char** args;
        Token* argsType;
        int nArgs;
        unsigned short* call;
        unsigned short ncall;
        
        /* Semantic control */
        unsigned short* def;		// Line of def

	    // Sizes
	    unsigned short ndef;
        struct symTable* env;

        bool _type;
    };
 } symbolEntry; 
  
 typedef struct symbolTable{ 
         char* scope;
         int len;        // Len of children 
         union symbolEntry* env;
         struct symbolTable *child, *parent;
 } symbolTable;

#endif