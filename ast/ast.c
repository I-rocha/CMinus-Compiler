#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

static void save(astNo* no, int deep, FILE* fd);
static void print(astNo* no, int deep);
static void printInfo(astNo* no, int deep);

astNo* astInit(){
	astNo* no;
	no = (astNo*)malloc(sizeof(astNo));
	no->label = BLANK;
	no->sibling = NULL;
	no->child = NULL;
	no->len_child = 0;

	return no;
}


int astCreateChild(astNo* no, int sz){
	if(!no)
		return -1;

	no->child = (astNo**)malloc(sz * sizeof(astNo*));
	no->len_child = sz;

	for(int i = 0; i < sz; i++)
		no->child[i] = NULL;
	
	return 1;
}

int astPutChild(astNo* no, astNo** target, int sz){
	int len = 0;
	
	if(!no || !target)
		return 0;

	// Allocates expected num of child
	no->child = (astNo**)malloc(sz * sizeof(astNo*));

	for(int i = 0; i < sz; i++){
		// Only existing targets
		if(!target[i])
			continue;
		

		// Assign and update len
		no->child[len++] = target[i];
	}

	// Deallocate if some target[i] was null
	no->child = (astNo**) realloc(no->child, len * sizeof(astNo*));
	no->len_child = len;

	return 1;
}

int astInsertSibling(astNo* no, astNo target){
	astNo* auxNo;

	// if null
	if(!no)
		return 0;

	auxNo = no;

	// look empty sibling
	while(auxNo->sibling)
		auxNo = auxNo->sibling;

	// allocate
	auxNo->sibling = (astNo*)malloc(sizeof(astNo));

	// insert
	*(auxNo->sibling) = target;

	return 1;
}

int astPutSibling(astNo* no, astNo** target, int len){
	// if null
	if(!no || !target)
		return 0;

	// Find empty sibling
	while(no->sibling)
		no = no->sibling;

	// Allocates (if exist) len sibling
	for(int i = 0; i < len; i++){

		// Check null
		if(!target[i])
			continue;

		// assign & update no
		no->sibling = target[i];
		no = no->sibling;
	}

	return 1;
}

astNo astNewNo(Token label, astNo** child, int len_child){
	astNo no;

	no.label = label;
	no.child = child;
	no.len_child = len_child;
	no.sibling = NULL;

	return no;
}

astNo* astCreateNo(Token label, astNo** child, int len_child){
	astNo* no;
	no = (astNo*)malloc(sizeof(astNo));

	no->label = label;
	no->child = child;
	no->len_child = len_child;
	no->sibling = NULL;

	return no;
}



int astDeepFree(astNo* no){
	if(!no)
		return 1;

	// Walk over tree
	if(no->child){
		for(int i = 0; i < no->len_child; i++){
			astDeepFree(no->child[i]);
		}
	}

	astDeepFree(no->sibling);

	// Free each child
	if(no->child){
		for(int i = 0; i < no->len_child; i++){
			free(no->child[i]);
		}
	}

	free(no->child);
	free(no->sibling);

	return 1;
}

int astFree(astNo* tree){
	if(!tree)
		return 1;

	if(astDeepFree(tree)){
		free(tree);
		return 1;
	}
	return -1;
}

void astPrintInfo(astNo* no){
	if(!no)
		return;

	printInfo(no, 0);
}

void astPrint(astNo* no){
	if(!no)
		return;

	print(no, 0);
}

int astSave(astNo* tree, char* path){
	FILE* fd;

	if(!path || !tree)
		return 0;

	if(!(fd = fopen(path, "w"))){
		printf("saveAST()\n");
		return -1;
	}
	fprintf(fd, "\t---------- BEGIN ----------\n");
	save(tree, 0, fd);
	fprintf(fd, "\t---------- END ----------\n");

	if(fd)
		fclose(fd);

	return 1;
}

/*
 * ########### PRIVATE ################ */
void printInfo(astNo* no, int deep){
	if(!no)
		return;

	for(int i = 0; i < deep; i++)
		printf("   ");

	printf("[Label, sibling, child]: [%s, %p, %p]\n", tokenStr(no->label), no->sibling, no->child);

	// Print deep inside
	for(int i = 0; i < no->len_child; i++)
		printInfo(no->child[i], deep + 1);

	printInfo(no->sibling, deep);
}

void print(astNo* no, int deep){
	if(!no)
		return;

	for(int i = 0; i < deep; i++)
		printf("   ");

	printf("[%s]\n", tokenStr(no->label));

	// Print deep inside
	for(int i = 0; i < no->len_child; i++)
		print(no->child[i], deep + 1);

	print(no->sibling, deep);
}

void save(astNo* no, int deep, FILE* fd){
	if(!no)
		return;

	for(int i = 0; i < deep; i++)
		fprintf(fd, "   ");

	fprintf(fd, "[%s]\n", tokenStr(no->label));

	// Print deep inside
	for(int i = 0; i < no->len_child; i++)
		save(no->child[i], deep + 1, fd);

	save(no->sibling, deep, fd);
}

char* tokenStr(Token tok){
	switch(tok){
		case BLANK:
			return "BLANK";
			break;
		
		case ALLOC_K:
			return "ALLOC";
			break;

		case ALLOC_ARRAY_K:
			return "ALLOC_ARRAY";
			break;

		case ARRAY_SIZE_K:
			return "ARRAY_SIZE";
			break;

		case INT_K:
			return "INT";
			break;
		
		case VOID_K:
			return "VOID";
			break;

		case FUN_K:
			return "FUN";
			break;

		case ARG_K:
			return "ARG_K";
			break;

		case ARG_ARRAY_K:
			return "ARG_ARRAY";
			break;

		case IF_K:
			return "IF";
			break;

		case WHILE_K:
			return "WHILE";
			break;

		case RETURN_K:
			return "RETURN";
			break;

		case ASSIGN_K:
			return "ASSIGN";
			break;

		case VAR_K:
			return "VAR";
			break;
		
		case VAR_ARRAY_K:
			return "VAR_ARRAY";
			break;

		case NUM_K:
			return "NUM";
			break;

		case CALL_K:
			return "CALL";
			break;

		case LEQ_K:
			return "LEQ";
			break;

		case LESS_K:
			return "LESS";
			break;

		case GEQ_K:
			return "GEQ";
			break;

		case GRAND_K:
			return "GRAND";
			break;

		case EQ_K:
			return "EQ";
			break;

		case DIFF_K:
			return "DIFF";
			break;

		case PLUS_K:
			return "PLUS";
			break;

		case MINUS_K:
			return "MINUS";
			break;

		case MULT_K:
			return "MULT";
			break;

		case DIV_K:
			return "DIV";
			break;

		default:
			return "UNKOWN";
	}
	return "";
}

/*
int main(int argc, char** argv){
	astNo* no;
	no = astCreateNo(NUM_K, NULL, 0);
	astNo* aux[] = {astCreateNo(MINUS_K, NULL, 0), astCreateNo(MULT_K, NULL, 0)};
	astPutChild(no, aux, 2);	
	astPrint(no);
	return 1;
}*/
