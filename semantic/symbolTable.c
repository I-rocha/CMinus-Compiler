#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolTable.h"

symbolTable* newScope(char* scopeName, symbolTable* parent){
    symbolTable* st;
    st = (symbolTable*)malloc(sizeof(symbolTable));
    st->scope = scopeName;
    st->size = 0;
    st->len = 0;
    st->env = NULL;
    st->child = NULL;
    st->parent = parent;

    return st;
}


void addChild(symbolTable* parent, symbolTable child){
    if(parent == NULL)
        return;

    child.parent = parent;
    parent->child = realloc(parent->child, ++parent->size * sizeof(symbolTable));
    parent->child[parent->size-1] = child;
}

void addEntry(symbolTable* table, symbolEntry entry){
    if(table == NULL) return;

    table->env = realloc(table->env, ++table->len * sizeof(symbolEntry));
    table->env[table->len-1] = entry;
}


symbolEntry* lookUp(symbolTable* st, char* name){
    symbolTable* tab;
    tab = st;
    while(tab != NULL){
        for(int i = 0; i < tab->len; i++){
            if(strcmp(name, tab->env[i].name) == 0){
                return &tab->env[i];
            }
        }
        tab = tab->parent;
    }
    return NULL;
}

symbolEntry* look(symbolTable* st, char* name){
    symbolTable* tab;
    tab = st;
    for(int i = 0; i < tab->len; i++){
        if(strcmp(name, tab->env[i].name) == 0){
            return &tab->env[i];
        }
    }
    return NULL;
}

void addDefinition(symbolEntry* se, unsigned short defLine){
    if(!se) return;
    se->def = (unsigned short*)realloc(se->def, ++se->ndef * sizeof(unsigned short));
    se->def[se->ndef-1] = defLine;
}

symbolEntry newEntry(char* name, Token type, unsigned short lineOfDefinition, int _sizeof){
    symbolEntry entry;
    entry.name = name;
    entry.type = type;
    entry.def = (unsigned short*) malloc(sizeof(unsigned short));
    entry.def[0] = lineOfDefinition;
    entry.ndef = 1;
    entry._isFunction = false;
    entry._sizeof = _sizeof;
}

// void main(){
//     symbolTable* head;
//     symbolTable* aux;
//     symbolEntry entry;
//     entry.name = strdup("var1");
    

//     head = newScope("GLOBAL", NULL);
//     aux = newScope("Fun1", head);
//     addEntry(aux, entry);
//     entry.name = strdup("varGlobal");
//     addEntry(head, entry);

//     printf("Variavel: %s\n", lookUp(aux, "varGlobal")->name);   
// }