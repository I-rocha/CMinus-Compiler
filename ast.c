#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

ast createNo(ttoken mytk){
	ast myNo;

	myNo = (ast)malloc(sizeof(struct ast));
	if(myNo == NULL){
		printf(AST_ERR_0);
		return NULL;
	}
	myNo->children = NULL;
	myNo->n_child = 0;
	myNo->tok = mytk;
	return myNo;

}

int childrenSpace(ast no, int n_child){
	if (no == NULL){
		printf(AST_ERR_1);
		return -1;
	}

	no->children = (ast*)calloc(n_child, sizeof(struct ast*));

	if(no->children == NULL){
		printf(AST_ERR_0);
		return -1;
	}
	no->n_child = n_child;
	return 1;
}

void printTree(ast no, int depth){
	int i=0;
	if(no == NULL)
		return;
	
	printSpace(depth);
	
	print(no->tok);
	printf("\n");
	if(no->n_child == 0) return;

	while(i < no->n_child)
		printTree(no->children[i++], depth+1);
	return;
}

void freeTree(ast no){
	int i=0;
	if(no == NULL)
		return;
	
	if(no->n_child != 0){ 
		while(i < no->n_child)
			freeTree(no->children[i++]);
	}
	free(no);
	return;
}


void printSpace(int nSpace){
	int txtPad = 0;
	txtPad = nSpace * TAB;
	for(int i=0; i<nSpace * TAB; i++)
		printf(" ");
}

void print(ttoken sym){
	if(sym == kmult)
		printf("mult");
	else if(sym == ksum)
		printf("sum");
	else if(sym == krel)
		printf("relacional");
	else if(sym == kparam)
		printf("param");
	else if(sym == NIL)
		printf("NIL");
	else
		printf("%d", sym);
}

/*
 *TODO: Remover
int main(int argc, char** argv){
	ast no = createNo(kc);
	
	childrenSpace(no, 2);
	no->children[0].tok = ka;
	no->children[1].tok = kb;

	printTree(no, 0);

	if(no != NULL){
		if(no->children != NULL)
			free(no->children);
		free(no);
	}

	return 1;
}*/
