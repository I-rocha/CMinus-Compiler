#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ast storeNo(char* noName, ast* children, int n_child){
	ast currNo;

	currNo = (ast)malloc(sizeof(ast*));
	currNo->name = strdup(noName);
	currNo->n_child = n_child;

	if(children == NULL){
		currNo->children = NULL;
	}

	else
	{
		currNo->children = (ast*)malloc(sizeof(children));/*TODO: verificar alocacao sobrescrita*/
		currNo->children = children;
	}

	return currNo;
}


int preorderWalk(ast no, int depth){
	unsigned int i = 0;

	if(no == NULL)
		return 1;

	printNested(no->name, depth * WHITE_SP);
	while(i < no->n_child){
		if(no->children[i] == NULL){
			printf(AST_ERR_0);
			return -1;
		}

		preorderWalk(no->children[i], 1+depth);
		i++;
	}
	return 1;
}


void printNested(char* msg, int ntab){
	for(int i = 0; i < ntab; i++)
		printf(" ");
	printf("%s\n", msg);
}

// Free no e seus descendentes
int freeDesc(ast no){
	unsigned int i =0;

	if(no == NULL)
		return 1;

	while(i < no->n_child){
		if(no->children[i] == NULL){
			printf(AST_ERR_0);
			return -1;
		}

		freeDesc(no->children[i]);
		i++;
	}
	
	if(no->children != NULL) free(no->children);
	free(no->name);
	free(no);
	no = NULL;
	return 1;
}

void freeA(ast no){
	if(no != NULL){
		if(no->name != NULL)
			free(no->name);
		if(no->children != NULL)
			free(no->children);
		free(no);
	}
}

int children_concat(ast chA, ast chB){
	int idxA, idxB, sz;

	if(chA == NULL){ 
		printf(AST_ERR_1);
		return -1;
	}

	idxB = 0;
	idxA = chA->n_child;					
	sz = chA->n_child + chB->n_child;	// Total size of vec1 + vec2

	while (idxA < sz){
		chA->children[idxA++] = chB->children[idxB++];
	}

	return 1;
}

int addChild(ast no, ast val){
	if(no->children == 0)
		no->children = (ast*)malloc(sizeof(ast));
	else
		no->children = (ast*)realloc(no->children, sizeof(ast) * (no->n_child+1));
	
	no->children[no->n_child] = val;	// Insere na posicao nova alocada
	no->n_child++;
	
	return 1;
	
}

// TODO: Remove this
/*
int main(int argc, char** argv){
	ast head, child1, child2;
	ast* children;
	char name[5] = "ABCD";
	
	child1 = (ast)malloc(sizeof(ast*));
	child2 = (ast)malloc(sizeof(ast*));

	child1->children = NULL;
	child2->children = NULL;


	child1->name = strdup("first");
	child2->name = strdup("second");

	children = (ast*)malloc(sizeof(ast));
	children[0] = child1;
	children[1] = child2;

	head = storeNo("ABCD", children, 2);
	preorderWalk(head, 0);

	freeDesc(head);
}*/
