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
		currNo->children = (ast*)malloc(sizeof(children));
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

		preorderWalk(no->children[i], ++depth);
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
void freeDesc(ast no){
	

}

// TODO: Remove this

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

	head = storeNo(name, children, 2);
	printf("HEAD: %s\n",head->children[1]->name);
	preorderWalk(head, 0);

	free(child1->name);
	free(child2->name);
	
	free(child1);
	free(child2);
	
	free(head->name);
	free(head);
	free(children);

}
