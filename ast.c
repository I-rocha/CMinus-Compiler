#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

ast createNo(grammID mytk){
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


void printTree(ast no, int depth){
	int i=0;
	if(no == NULL)
		return;

	printSpace(depth);
	print(no->tok);

	if(no->n_child == 0) return;

	while(i < no->n_child)
		printTree(no->children[i++], depth+1);

	return;
}


void printSpace(int nSpace){
	int txtPad = 0;
	txtPad = nSpace * TAB;
	for(int i=0; i<nSpace * TAB; i++)
		printf(" ");
}


void print(grammID sym){
	if(sym == kact) printf("act");
	else if(sym == karg_lista) printf("arg_lista");
	else if(sym == kterm) printf("term");
	else if(sym == kmult) printf("mult");
	else if(sym == ksum) printf("sum");
	else if(sym == kfact) printf("fact");
	else if(sym == ksoma_exp) printf("soma_exp");
	else if(sym == krel) printf("relacional");
	else if(sym == ksimple_exp)	printf("simple_exp");
	else if(sym == kvar)	printf("var");
	else if(sym == kexp)	printf("exp");
	else if(sym == kretorno_decl)	printf("retorno_decl");
	else if(sym == kiteracao_decl)	printf("iteracao_decl");
	else if(sym == kselecao_decl)	printf("selecao_decl");
	else if(sym == kexp_decl)	printf("exp_decl");
	else if(sym == kstatement_lista)	printf("statement_lista");
	else if(sym == klocal_decl)	printf("local_decl");
	else if(sym == kcomposto_decl)	printf("composto_decl");
	else if(sym == kparam) printf("param");
	else if(sym == kparam_lista)	printf("param_lista");
	else if(sym == kparams)	printf("params");
	else if(sym == kfun_decl)	printf("fun_decl");
	else if(sym == ktipo_esp)	printf("tipo_esp");
	else if(sym == kvar_decl)	printf("var_decl");
	else if(sym == kdecl_lista)	printf("decl_lista");
	else if(sym == NIL)	printf("NIL");
	else if(sym == terminal)	printf("terminal");
	else printf(AST_ERR_2);
	printf("\n");
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
