#ifndef REGBANK_H
#define REGBANK_H

typedef struct RCell rcell;

struct RCell{
    char* var;
    short reg;
    short isArray;
    int idx;

    rcell* prox;
};


void initRegManager();

short linkRegister(char* var);
short getRegister(char* var);
short linkReg_(char* var, short isArray, int idx);
short getReg_(char* var, short isArray, int idx);

void cleanFilled();
void printRegManager();

#endif
