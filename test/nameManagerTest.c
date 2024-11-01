#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lexical/nameManager.h"

ArrayString nameManager;

void factoryGroupRepetition();
void factorySimpleGroup();
void base();
void hardFree();
int addNameTest();
int addLenTest();
int multipleAddNameTest();
int lenFreeTest();
int freeTest();


void hardFree(){
    for(int i = 0; i < nameManager.len; i++){
        free(nameManager.names[i]);
        nameManager.names[i] = NULL;
    }
    nameManager.len = 0;
}

int addNameTest(){
    nmAdd("Pedro");
    if(strcmp(nameManager.names[0], "Pedro") != 0){
        printf("ERROR: Name manager add\n");
        hardFree();
        return 0;
    }
    hardFree();
    return 1;
}

int addLenTest(){
    factorySimpleGroup();
    if(nameManager.len != 3){
        printf("ERROR: Name manager len");
        hardFree();
        return 0;
    }
    hardFree();
    return 1;
}

int multipleAddNameTest(){
    factoryGroupRepetition();

    if(strcmp(nameManager.names[3], "Jose") != 0){
        printf("ERROR: Name manager multipleAdd\n");
        hardFree();
        return 0;
    }
    hardFree();
    return 1;
}

int lenFreeTest(){
    factoryGroupRepetition();
    nmClean();
    if(nameManager.len != 0){
        printf("ERROR: Name manager lenFreeTest\n");
        hardFree();
        nmInit();
        return 0;
    }
    nmInit();
    hardFree();
    return 1;
}

int freeTest(){
    factoryGroupRepetition();
    nmClean();
    if(nameManager.names){
        printf("ERROR: Name Manager freeTest\n");
        hardFree();
        nmInit();
        return 0;
    }
    hardFree();
    nmInit();
    return 1;
}

void base(){
    nmInit();
}

void factorySimpleGroup(){
    nmAdd("Pedro");
    nmAdd("Joao");
    nmAdd("Maria");
}

void factoryGroupRepetition(){
    nmAdd("Pedro");
    nmAdd("Joao");
    nmAdd("Maria");
    nmAdd("Pedro");
    nmAdd("Maria");
    nmAdd("Jose");
}

void main(){
    printf("Assertion starting\n");
    int point = 0;
    base();
    point += addNameTest();
    point += addLenTest();
    point += multipleAddNameTest();
    point += lenFreeTest();
    point += freeTest();

    printf("Assertion %d/5\n", point);
    nmClean();
}