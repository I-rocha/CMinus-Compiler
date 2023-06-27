#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>	// variadic

#include "mnemonic.h"
#include "../utils.h"

static int lineno = -1;
static instruction_subset formatI, formatII, formatIII, format_set[SUBSET_SZ];
static operation_t o1[] = {add, sub, AND, OR, NOT, XOR, less, grand, eq, neq, leq, geq, shiftL, shiftR};
static operation_t o2[] = {mvi, addi, subi, ANDi, ORi, NOTi, XORi, lessi, grandi, eqi, neqi, leqi, geqi, lup, ldown};
static operation_t o3[] = {mv, jump, jal, jc, branch, bal, bc, sw, lw, get, print, NOP, STOP};
static char * mnemonic[] = {
	"add\0",
	"sub\0",
	"addi\0",
	"subi\0",
	"AND\0",
	"OR\0",
	"NOT\0",
	"XOR\0",
	"ANDi\0",
	"ORi\0",
	"NOTi\0",
	"XORi\0",
	"shiftL\0",
	"shiftR\0",
	"less\0",
	"grand\0",
	"eq\0",
	"neq\0",
	"leq\0",
	"geq\0",
	"lessi\0",
	"grandi\0",
	"eqi\0",
	"neqi\0",
	"leqi\0",
	"geqi\0",
	"mv\0",
	"mvi\0",
	"sw\0",
	"lw\0",
	"lup\0",
	"ldown\0",
	"jump\0",
	"jal\0",
	"jc\0",
	"branch\0",
	"bal\0",
	"bc\0",
	"get\0",
	"print\0",
	"NOP\0",
	"STOP\0",
	"UNKNOWN\0"
};

static int getFormat(operation_t* op);
static int setMeta(instruction* instr);
static char* int2Bin(int dec, int nbits);

static memmory ram;

void printRam(){
	char* str;
	for(int i = 0; i < ram.len; i++){
		str = instruction2String(&ram.instr[i]);
		printf("%s\n", str);
	}
	return;
}

void initGlobal(){
	formatI.operation = o1;	
	formatI.len = F1_LEN;
	formatI.formatID = FORMAT_I;

	formatII.operation = o2;
	formatII.len = F2_LEN;
	formatII.formatID = FORMAT_II;

	formatIII.operation = o3;
	formatIII.len = F3_LEN;
	formatIII.formatID = FORMAT_III;

	format_set[0] = formatI;
	format_set[1] = formatII;
	format_set[2] = formatIII;

	lineno = -1;

	ram.len = 0;
	ram.instr = NULL;
}

instruction* newInstruction(operation_t operation, ...){
	instruction this_instruction;
	instruction *instr = &this_instruction;

	// Variadic arguments
	va_list args;
	va_start(args, operation);

	// adding operation
	instr->operation = operation;
	
	// set META
	if(setMeta(instr) == -1)
		printf("Error assigning META. (%s)\n", __func__);

	// Check format of instruction and assign attributes accordingly
	switch(instr->formatID){
	case FORMAT_I:
		instr->r1 = va_arg(args, int);	// r1
		instr->r2 = va_arg(args, int);	// r2
		va_arg(args, int);				// padding. This fields is not processed but exists
		instr->shamt = va_arg(args, int); // shamt
		break;

	case FORMAT_II:
		instr->r1 = va_arg(args, int);	//r1
		instr->immediate = va_arg(args, int); // immediate
		break;

	case FORMAT_III:
		instr->r1 = va_arg(args, int); 	// r1
		instr->r2 = va_arg(args, int); 	// r2
		instr->desl = va_arg(args, int); // desl

		break;
	}

	// End variadic
	va_end(args);


	ram.instr = (instruction*)realloc(ram.instr, sizeof(instruction) * (ram.len + 1));
	allocateValidator((void**)&ram.instr, REALLOC_VALIDATE);
	ram.instr[ram.len++] = *instr;
	return &ram.instr[ram.len];
}

// TODO: str_instr must be dealocated somewhere
char* instruction2BinStr(instruction* instr){
	char *str_opcode, *str_funct, *str_r1, *str_r2, *str_shamt, *str_immediate, *str_desl, *str_padding;
	char *str_instr, str_partial[32] = "";

	str_instr = (char*)malloc(sizeof(char) * (BIT_ARCH + 1));
	if(!str_instr){
		printf("Error: string allocation required not allocated. (instruction2BinStr)\n");
		return NULL;
	}
	str_instr[0] = '\0';


	// NOTE: Both fields below are present for any format
	str_opcode = int2Bin(instr->opcode, 6);
	str_r1 = int2Bin(instr->r1, 5);

	// Putting together
	strcat(str_instr, str_opcode);
	strcat(str_instr, str_r1);

	// Rest of the fields are format-specific
	switch(instr->formatID){
	case FORMAT_I:
		str_r2 = int2Bin(instr->r2, 5);
		str_shamt = int2Bin(instr->shamt, 5);
		str_padding = int2Bin(0, 6);		// This field is ignored
		str_funct = int2Bin(instr->funct, 5);

		strcat(str_partial, str_r2);
		strcat(str_partial, str_shamt);
		strcat(str_partial, str_padding);
		strcat(str_partial, str_funct);

		break;

	case FORMAT_II:
		str_immediate = int2Bin(instr->immediate, 16);
		str_funct = int2Bin(instr->funct, 5);

		strcat(str_partial, str_immediate);
		strcat(str_partial, str_funct);
		break;

	case FORMAT_III:
		str_r2 = int2Bin(instr->r2, 5);
		str_desl = int2Bin(instr->desl, 16);

		strcat(str_partial, str_r2);
		strcat(str_partial, str_desl);
		break;
	}
	strcat(str_instr, str_partial);
	printf("Instrucao:\n%s\n\n", str_instr);

	return str_instr;
}

// TODO: Need to dealocate str in some place
char* int2Bin(int dec, int nbits){
	int bit, mask, aux_mask;
	char *str;
	
	// Mask will help to eliminate the most significant bit
	mask = 63;	// b'111111
	str = (char*)malloc(sizeof(char) * (nbits + 1));
	
	if(!str){
		printf("ERROR: str required not allocated (int2Bin)\n");
		return NULL;
	}
	str[0] = '\0';

	// Loop over each bit
	for(int i = nbits-1; i >= 0; i--){
		// Isolate a bit and use unary compare

		bit = dec >> i;
		aux_mask = mask << i;

		// Obtain most significant bit
		if(bit & 1)
			strcat(str, "1");
		else
			strcat(str, "0");

		// Removes most significant bit
		dec = dec & (~aux_mask);
	}
	return str;
}

// TODO: Need to dealocate str in some place
char* instruction2String(instruction* instr){
	char *str, *operation;

	str = (char*)malloc(sizeof(char) * (BIT_ARCH+1));
	if(!str){
		printf("Error: String allocation denied. (instruction2String)\n");
		return NULL;
	}
	str[0]='\0';

	operation = operation2String(&instr->operation);

	switch(instr->formatID){
	case FORMAT_I:
		sprintf(str, "%s $r%d, $r%d, %d", operation, instr->r1, instr->r2, instr->shamt);
		break;
	case FORMAT_II:
		sprintf(str, "%s $r%d, %d", operation, instr->r1, instr->immediate);
		
		break;
	case FORMAT_III:
		sprintf(str, "%s $r%d, $r%d, %d", operation, instr->r1, instr->r2, instr->desl);
		break;
	}
	return str;
}

int getFormat(operation_t* op){
	instruction_subset subset;

	if(op == NULL)	
		return -1;

	for(int i_subset = 0; i_subset < SUBSET_SZ; i_subset++){
		subset = format_set[i_subset];

		for(int i = 0; i < subset.len; i++){
			if(*op == subset.operation[i]){
				return subset.formatID;
			}
		}
	}
	return -1;
}


char* operation2String(operation_t* operation){
	if(*operation >= N_OPERATIONS){
		return "UNKNOWN";
	}
	else
		return mnemonic[*operation];
}

int getLine(){
	return lineno;
}

int setMeta(instruction* instr){
	int format = -1;
	format = getFormat(&(instr->operation));
	instr->formatID = format;

	switch(instr->operation){
	case add:
		instr->funct = 1;
		instr->opcode = 1;
		break;

	case sub:
		instr->funct = 2;
		instr->opcode = 1;

		break;
	case addi:
		instr->funct = 3;
		instr->opcode = 1;

		break;
	case subi:
		instr->funct = 4;
		instr->opcode = 1;

		break;
	case AND:
		instr->funct = 1;
		instr->opcode = 2;

		break;
	case OR:
		instr->funct = 2;
		instr->opcode = 2;

		break;
	case NOT:
		instr->funct = 3;
		instr->opcode = 2;

		break;
	case XOR:
		instr->funct = 4;
		instr->opcode = 2;

		break;
	case ANDi:
		instr->funct = 5;
		instr->opcode = 2;

		break;
	case ORi:
		instr->funct = 6;
		instr->opcode = 2;

		break;
	case NOTi:
		instr->funct = 7;
		instr->opcode = 2;

		break;
	case XORi:
		instr->funct = 8;
		instr->opcode = 2;

		break;
	case shiftL:
		instr->funct = 9;
		instr->opcode = 2;

		break;
	case shiftR:
		instr->funct = 10;
		instr->opcode = 2;

		break;
	case less:
		instr->funct = 1;
		instr->opcode = 3;

		break;
	case grand:
		instr->funct = 2;
		instr->opcode = 3;

		break;
	case eq:
		instr->funct = 3;
		instr->opcode = 3;

		break;
	case neq:
		instr->funct = 4;
		instr->opcode = 3;

		break;
	case leq:
		instr->funct = 5;
		instr->opcode = 3;

		break;
	case geq:
		instr->funct = 6;
		instr->opcode = 3;

		break;
	case lessi:
		instr->funct = 7;
		instr->opcode = 3;

		break;
	case grandi:
		instr->funct = 8;
		instr->opcode = 3;
		break;

	case eqi:
		instr->funct = 9;
		instr->opcode = 3;
		break;

	case neqi:
		instr->funct = 10;
		instr->opcode = 3;
		break;

	case leqi:
		instr->funct = 11;
		instr->opcode = 3;
		break;

	case geqi:
		instr->funct = 12;
		instr->opcode = 3;
		break;

	case mv:
		instr->opcode = 4;
		break;

	case mvi:
		instr->opcode = 5;
		break;

	case sw:
		instr->opcode = 6;
		break;

	case lw:
		instr->opcode = 7;
		break;

	case lup:
		instr->opcode = 8;
		break;

	case ldown:
		instr->opcode = 9;
		break;

	case jump:
		instr->opcode = 10;
		break;

	case jal:
		instr->opcode = 11;
		break;

	case jc:
		instr->opcode = 12;
		break;

	case branch:
		instr->opcode = 13;
		break;

	case bal:
		instr->opcode = 14;
		break;

	case bc:
		instr->opcode = 15;
		break;

	case get:
		instr->opcode = 16;
		break;

	case print:
		instr->opcode = 17;
		break;

	case NOP:
		instr->opcode = 0;
		break;

	case STOP:
		instr->opcode = 63;
		break;

	default:
		return -1;
	}
	instr->line = ++lineno;
	return 1;
}
