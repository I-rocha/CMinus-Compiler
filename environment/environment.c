#include <stdio.h>
#include <stdlib.h>

#include "environment.h"
#include "../cgen/cgen.h"
#include "mnemonic.h"

#define MEM_SZ 1000


cell memory[MEM_SZ];

void envInitGlobal(){
	initGlobal();
}

void initFunCode(quad* head){
	int idx = 0;
	quad* curr;
	curr = head;
	while(curr != NULL){
		switch(curr->op){
		case FUN_C:
			memory[idx++].code = curr;
			break;
		default:
			break;
		}
		curr = curr->next;
	}
	
	for(int j = 0; j < idx; j++){
		printf("%s", memory[j].code->arg2);
		printf("\n");
	}

}

void runEnv(){
	// Set sp to 0
	printf("set sp to 0\n");
	// set fp to 0
	printf("set fp to 0\n");



}


void test(){
	char* str, *str2;
	instruction instr;
	instr = newInstruction(add, 5, 3, 0, 0);
	str = instruction2BinStr(&instr);
	str2 = instruction2String(&instr);
	printf("instrucao (assembly): %s\n", str);
	printf("instrucao (mnemonicos): %s\n", str2);

	// printf("instrucao: %s\n", str);

}