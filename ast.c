#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "file_log.h"

ast createNo(grammID mytk, int line){
	ast myNo;

	myNo = (ast)malloc(sizeof(struct ast));
	if(myNo == NULL){
		printf(AST_ERR_0);
		return NULL;
	}
	myNo->children = NULL;
	myNo->n_child = 0;
	myNo->tok = mytk;
	myNo->termTok = NONE;
	myNo->line = line;
	return myNo;

}

ast createNoTerminal(terminalID mytk, int line){
	ast myNo;

	myNo = (ast)malloc(sizeof(struct ast));
	if(myNo == NULL){
		printf(AST_ERR_0);
		return NULL;
	}
	myNo->children = NULL;
	myNo->n_child = 0;
	myNo->tok = terminal;
	myNo->termTok = mytk;
	myNo->line = line;
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
	if(no->name != NULL){
		free(no->name);
		no->name = NULL;
	}
	free(no);
	no = NULL;
	return;
}


void printTree(ast no, int depth){
	int i=0;
	if(no == NULL)
		return;

	printSpace(depth);
	print(no);
	printf("\n");

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


void print(ast no){
	grammID sym;
	terminalID tid;

	sym = no->tok;
	tid = no->termTok;

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
	else if(sym == terminal){
		if(tid == kNUM) printf("%d", no->val);
		else printf("%s", no->name);
	}
	else printf(AST_ERR_2);
}

/*	FILES */

void fprintTree(ast no, int depth){
	int i=0;
	if(no == NULL)
		return;

	fprintSpace(depth);
	fprint(no);
	fprintf(p_log, "\n");

	if(no->n_child == 0) return;

	while(i < no->n_child)
		fprintTree(no->children[i++], depth+1);

	return;
}


void fprintSpace(int nSpace){
	int txtPad = 0;
	txtPad = nSpace * TAB;
	for(int i=0; i<nSpace * TAB; i++)
		fprintf(p_log, " ");
}


void fprint(ast no){
	grammID sym;
	terminalID tid;

	sym = no->tok;
	tid = no->termTok;

	if(sym == kact) fprintf(p_log, "act");
	else if(sym == karg_lista) fprintf(p_log, "arg_lista");
	else if(sym == kterm) fprintf(p_log, "term");
	else if(sym == kmult) fprintf(p_log, "mult");
	else if(sym == ksum) fprintf(p_log, "sum");
	else if(sym == kfact) fprintf(p_log, "fact");
	else if(sym == ksoma_exp) fprintf(p_log, "soma_exp");
	else if(sym == krel) fprintf(p_log, "relacional");
	else if(sym == ksimple_exp)	fprintf(p_log, "simple_exp");
	else if(sym == kvar)	fprintf(p_log, "var");
	else if(sym == kexp)	fprintf(p_log, "exp");
	else if(sym == kretorno_decl)	fprintf(p_log, "retorno_decl");
	else if(sym == kiteracao_decl)	fprintf(p_log, "iteracao_decl");
	else if(sym == kselecao_decl)	fprintf(p_log, "selecao_decl");
	else if(sym == kexp_decl)	fprintf(p_log, "exp_decl");
	else if(sym == kstatement_lista)	fprintf(p_log, "statement_lista");
	else if(sym == klocal_decl)	fprintf(p_log, "local_decl");
	else if(sym == kcomposto_decl)	fprintf(p_log, "composto_decl");
	else if(sym == kparam) fprintf(p_log, "param");
	else if(sym == kparam_lista)	fprintf(p_log, "param_lista");
	else if(sym == kparams)	fprintf(p_log, "params");
	else if(sym == kfun_decl)	fprintf(p_log, "fun_decl");
	else if(sym == ktipo_esp)	fprintf(p_log, "tipo_esp");
	else if(sym == kvar_decl)	fprintf(p_log, "var_decl");
	else if(sym == kdecl_lista)	fprintf(p_log, "decl_lista");
	else if(sym == NIL)	fprintf(p_log, "NIL");
	else if(sym == terminal){
		if(tid == kNUM) fprintf(p_log, "terminal: %d", no->val);
		else fprintf(p_log, "terminal: %s", no->name);
	}
	else printf(AST_ERR_2);
}
