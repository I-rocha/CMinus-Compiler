#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbolTable.h"
#include "../lexical/nameManager.h"
#include "../ast/ast.h"

int n_err = 0;
symbolTable* head;
symbolTable* currScope;

void semanticStart(){
    head = newScope(nmAdd("GLOBAL"), NULL);
    currScope = head;
}


void varDefinition(astNo* root){
    // Validation
    if(!root->child){
        printf("ERR: Trying to access child of type declaration leading to NULL ast from semantic\n");
        return;
    }

    // Get info
    astNo* child = root->child[0];
    Token typeDef = root->label;
    Token categorie = child->label;
    char* name = child->instance;

    // Look for previous definition
    symbolEntry* se = look(currScope, name);
    if(se) {
        addDefinition(se, child->line);
        return;
    }

    // Size of variable type
    int size = (child->child) ? atoi(child->child[0]->instance) : 1; 

    // Add new entry with infos
    symbolEntry data = newEntry(name, typeDef, child->line, size);
    addEntry(currScope, data);
}

// void functionDefinition(){
    
// }


// TODO: Implementar
astNo* walkDefinitions(astNo* root){
    if (!root) return;

    else if(root->label == INT_K || root->label == VOID_K){
        varDefinition(root);
    }
}

void semantic(astNo* root){

    astNo* curr;
    curr = root;
    semanticStart();
    walkDefinitions(curr);
}