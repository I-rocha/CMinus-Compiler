#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "parser.tab.h"
#include "lexical/symtab.h"
#include "semantic/semantic.h"
#include  "cgen/cgen.h"
#include "environment/environment.h"
#include "environment/mnemonic.h"
#include "GLOBALS.h"

extern FILE* yyin;
FILE* fl;

astNo* astTree;
extern symTable *headEnv;
int main(int argc, char** argv){
	FILE *fp;
	memmory* mem;
	char fpath[100] = "";
	char outpath[100] = "" ;
	
	if(argc <=1){
		printf("Please follow pattern: <exe> <file>\n");
		return 0;
	}

	if(!(fp = fopen(argv[1], "r"))){
		printf("Error openning file of argument\n");
		return 0;
	}
	
	if(!(fl = fopen("output/lexical.txt", "w"))){
		printf("Error oppening file of lexical output\n");
		return 0;
	}

	strcpy(outpath, OUTPUT_PATH);
	// strcat(outpath, argv[1]);
	strcat(outpath, "/");

	yyin = fp;

	// Parser + Lexical
	yyparse();
	strcpy(fpath, outpath);
	strcat(fpath, AST_F);
	astSave(astTree, fpath);

	/*	Symbol Table	*/
	semantic(astTree);

	strcpy(fpath, outpath);
	strcat(fpath, SYMT_F);
	symTSave(headEnv, fpath);

	// Intermediate Code
	quad* code;
	code = gen(astTree);
	if(PRINT_CGEN)
		printQuad(code);

	strcpy(fpath, outpath);
	strcat(fpath, CGEN_F);
	saveCI(code, fpath);
	
	/* ENVIRONMENT */
	
	envInitGlobal();
	mem = toAssembly(code);
	if(PRINT_MEM)
		printMem(mem);

	strcpy(fpath, outpath);
	strcat(fpath, BIN_F);
	saveBin(fpath);

	strcpy(fpath, outpath);
	strcat(fpath, ASSEMBLY_F);
	saveAssembly(fpath);

	strcpy(fpath, outpath);
	strcat(fpath, BIN_QUARTUS_F);
	saveBinQuartus(fpath);
	
	// Close files
	if(fp)
		fclose(fp);
	if(fl)
		fclose(fl);

	// Free pointers
	astFree(astTree);

	printf("------ END PROCESSING --------\n");	
	return 1;
}