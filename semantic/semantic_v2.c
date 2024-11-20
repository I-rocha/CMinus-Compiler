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

// TODO: Implementar
// astNo* walkTree(astNo* root){

// }

void semantic(astNo* root){
    semanticStart();
    // walkTree(root);
}