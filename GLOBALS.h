#ifndef GLOBALS_H
#define GLOBALS_H


// Addresses to md
#define MEM_SZ 300	// size of memmory
#define MEM_BASIS 4 // Basis of mem
#define VISITOR_FLAG_ADDR 0
#define FINISH_FLAG_ADDR 1
#define SP_ADDR 2
#define INPUT_ADDR 3
#define OUTPUT_ADDR 4

// Registers
#define ntemps 22 // Number of temporary
#define rj 31	// Register jump
#define rf 30	// register flag
#define sp 29	// stack pointer
#define fp$ 28	// frame pointer
#define oa 27	// override aux
#define rd 26	// return data
#define ra1$ 25   // Arg1 register
#define ra2$ 24   // Arg2 register
#define rr$ 23    // jt first reg
#define rt$ 22    // jr second reg

// About Processor
#define BIT_ARCH 32
#define N_OPERATIONS 53

// Pre-defined Functions
#define MAINF "main"
#define INPUTF "input"
#define OUTPUTF "output"
#define	ISDIRF "isDir"
#define ISACTIVEF "isActive"
#define GETBYTEF "getByte"
#define GETNFILESF "getNFiles"
#define SHIFTLBYTEF "shiftLByte"
#define SHIFTRBYTEF "shiftRByte"
#define GETADDRF "getAddr"
#define DISPLAYBYTEF "displayByte"
#define GETSIZENAMEF "getSizeName"
#define GETSIZEPAYLOADF "getSizePayload"
#define WRITEINSTRUCTIONF "writeInstruction"
#define RUNF "run"
#define SETBASISF "setBasis"
#define RUNCHRONOF "runChrono"
#define HASFINISHEDF "hasFinished"

// Paths/Files
#define OUTPUT_PATH "output/"
#define AST_F "ast"
#define BIN_F "bin"
#define BIN_QUARTUS_F "bin_quartus"
#define CGEN_F "cgen"
#define LEXICAL_F "lexical"
#define SYMT_F "symbolTable"
#define ASSEMBLY_F "assembly"

// Options
#define PRINT_MEM 0
#define PRINT_CGEN 0

#endif