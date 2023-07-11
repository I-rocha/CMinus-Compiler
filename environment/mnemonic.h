#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <stdarg.h>

#define SUBSET_SZ 3
#define FORMAT_I 0
#define FORMAT_II 1
#define FORMAT_III 2
#define F1_LEN 14
#define F2_LEN 15
#define F3_LEN 13

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
	STOP,
	UNKNOWN
}operation_t;

typedef struct{
	// Meta
	int formatID;	// automatically set by operation
	int opcode;		// automatically set by operation
	int funct;   // automatically set by operation
	int line;	// automatically set by operation

	// User-like
	operation_t operation;
	int immediate;
	int shamt;
	int r1;
	int r2;
	int* desl;

}instruction;

typedef struct{
	unsigned int len;

	instruction* instr;
}memmory;

// defines formatI, formatII, formatIII
typedef struct {
	operation_t* operation;
	int len;
	int formatID;
}instruction_subset;

// Public
void initGlobal();
memmory* newMem();
memmory* mergeInstructions(memmory* mem1, memmory* mem2);
instruction* newInstruction(memmory* mem, operation_t operation, ...);
instruction* getInstruction(memmory* mem, int idx);
char* instruction2BinStr(instruction* instr);
char* instruction2String(instruction* instr);
char* instruction2StringPretty(instruction* instr);
char* operation2String(operation_t* operation);
void printMem(memmory* mem);
int getLine();

#endif