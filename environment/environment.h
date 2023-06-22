#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdarg.h>
#include "../cgen/cgen.h"


typedef struct {
	char item;
} registry;

typedef struct {
	char item;
} glob;

typedef union{
	quad* code;
	glob global;
	registry activation;
} cell;

typedef enum{
	add = 0,
	sub,
	addi,
	subi,
	AND,
	OR,
	NOT,
	XOR,
	ANDi,
	ORi,
	NOTi,
	XORi,
	shiftL,
	shiftR,
	less,
	grand,
	eq,
	neq,
	leq,
	geq,
	lessi,
	grandi,
	eqi,
	neqi,
	leqi,
	geqi,
	mv,
	mvi,
	sw,
	lw,
	lup,
	ldown,
	jump,
	jal,
	jc,
	branch,
	bal,
	bc,
	get,
	print,
	NOP,
	STOP
}operation_t;

// defines formatI, formatII, formatIII
typedef struct {
	operation_t* operation;
	int len;
	int formatID;
}instruction_subset;


typedef struct{
	// Meta
	int formatID;	// automatically set by operation
	int opcode;		// automatically set by operation
	int funct;   // automatically set by operation

	// User-like
	operation_t operation;
	int immediate;
	int shamt;
	int r1;
	int r2;
	int desl;

}instruction;

void initFunCode(quad* head);
void initGlobal();
int envFormat(operation_t* op);
int setMeta(instruction* instr);
instruction newInstruction(operation_t operation, ...);
char* int2bin(int dec, int nbits);
char* instruction2binStr(instruction* instr);
char* instruction2String(instruction* instr);
void test();

#endif