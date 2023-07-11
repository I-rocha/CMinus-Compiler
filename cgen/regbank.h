#ifndef REGBANK_H
#define REGBANK_H

#define NREG 26

typedef struct rprec{
	int idx;
	char* var;
	struct rprec* prox;
}rprec;

typedef struct rlist{
	rprec* reg;
	struct rlist* prox;
}rlist;

void regBankInit();
int linkReg(char* var);
int getReg(char*var);
char* getVar(int reg);
int cleanFilled();
void printFilled();


#endif
