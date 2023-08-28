#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regbank.h"
#include "../GLOBALS.h"

rcell *first, *last;

static short bindEquals(rcell* reg, char* var, short isArray, int idx);

void initRegManager(){
    rcell* aux;

    first = (rcell*)malloc(sizeof(rcell));
    first->var = NULL;
    first->reg = 0;
    first->isArray = -1;
    first->idx = -1;
    first->prox = NULL;
    aux = first;

    for(int i = 1; i < ntemps; i++){
        aux->prox = (rcell*)malloc(sizeof(rcell));
        aux = aux->prox;

        aux->var = NULL;
        aux->idx = -1;
        aux->isArray = -1;
        aux->reg = i;
    }
    aux->prox = NULL;
    last = aux;
}

short linkRegister(char* var){
    return linkReg_(var, 0, 0);
}
short getRegister(char* var){
    return getReg_(var, 0, 0);
}

short linkReg_(char* var, short isArray, int idx){
    rcell* aux;

    first->var = var;
    first->isArray = isArray;
    first->idx = idx;

    aux = first->prox;
    first->prox = NULL;
    last->prox = first; // first to last
    last = last->prox;  // new last
    first = aux;    // new first
    return last->reg;
}

short getReg_(char* var, short isArray, int idx){
    rcell* aux;
    aux = first;

    for(int i = 0; i < ntemps; i++){
        if(bindEquals(aux, var, isArray, idx) > 0)
            return aux->reg;
       
        aux = aux->prox;
    }
    return -1;
}

void cleanFilled(){
    rcell* aux;
    aux = first;

    for(int i = 0; i <ntemps; i++){
        aux->var = NULL;
        aux = aux->prox;
    }
}

static short bindEquals(rcell* reg, char* var, short isArray, int idx){
    if(!reg)
        return -1;

    if(reg->var == NULL || var == NULL)
        return -1;

    if( ((reg->isArray == 1) && (isArray != 1)) ||
        ((reg->isArray != 1) && (isArray == 1))
        )
        return -1;
    
    if(strcmp(var, reg->var) == 0){
        if(isArray){
            if(reg->idx == idx)
                return 1;
        }
        else
            return 1;
    }
    return -1;
}

void printRegManager(){
    rcell *aux;
    char* var_str;
    char* array_str;

    aux = first;
    for(int i = 0; i < ntemps; i++){
        var_str = (aux->var != NULL)? aux->var : strdup("nil");

        if(aux->isArray == 1){
            sprintf(array_str, "[%d]", aux->idx);
        }
        else
            array_str = strdup("");           
        
        printf("[%d] -- %s%s\n", aux->reg, var_str, array_str);
        aux = aux->prox;
    }
}

// void main(){
//     printf("working\n");
//     initRegManager();
//     printf("Before print\n");
//     linkReg_(strdup("var1"), 1, 0);
//     linkReg_(strdup("var2"), 1, 5);
//     linkReg_(strdup("var3"), 0, 0);
//     printRegManager();
//     printf("Found %d\n", getReg_(strdup("var2"), 1, 0));
//     printf("Found %d\n", getReg_(strdup("var2"), 1, 5));
//     printRegManager();

//     cleanFilled();
// }