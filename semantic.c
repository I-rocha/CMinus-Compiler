//	C	//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//	My	//
#include "ast.h"
#include "semantic.h"

void table(ast root){
	attr aux;
	aux.type = NULL;
	aux.name = NULL;
	aux.scope = NULL;
	aux.var_func = -1;

	getTokens(root, &aux, GLOBAL);	
}


void getTokens(ast root, attr* currAttr, char* currScope){
	int i = 0;
	char* aux;

	if(root == NULL)
		return;

//	printf("nchild: %d ", root->n_child);
//	print(root);
	
	if(root->tok == terminal) return;

	
	/*	Not terminals	*/

	/* match fun decl*/
	if(root->tok == kfun_decl){						// tipo_esp ID (params) composto-decl 
		m_fun_decl(root, currAttr, currScope);
	
		/* Inside function*/;
		getTokens(root->children[3], currAttr, currScope);	// Child params	with updated scope
		getTokens(root->children[5], currAttr, currScope);	// Child composto-decl with updated scope

	//	free(currScope);
		currScope = strdup(GLOBAL);	// Global scope after function
		return;
	}

	/* match var_decl*/
	else if(root->tok == kvar_decl){	// tipo_esp ID ; | tipo_esp ID [NUM];
		m_var_decl(root, currAttr, currScope);
		return;
	}
	
	/* match varf_decl*/
	else if(root->tok == kparam){	// tipo_esp ID | tipo_esp ID []
		m_varf_decl(root, currAttr, currScope);
		return;
	}	
	
	/* match fun_act*/
	else if(root->tok == kact){	// ID (args)
		m_fun_act(root, currAttr, currScope);
		getTokens(root->children[2], currAttr, currScope);
		return;
	}
	
	/* match var*/
	else if(root->tok == kvar){	// ID | ID[exp]	
		m_var(root, currAttr, currScope);
		if(root->n_child > 1) getTokens(root->children[2], currAttr, currScope);
		return;
	}

	// Run tree
	while(i < root->n_child){
		getTokens(root->children[i++], currAttr, currScope);
	}

	return;

}


void m_fun_decl(ast root, attr* currAttr, char* currScope){
	// tipo_esp ID (params) composto-decl 
	printf("1\n");
	
	// Attr
	currAttr->type = strdup(nextTerminal(root->children[0])->name);	// Tipos especificador
	currAttr->name = strdup(nextTerminal(root->children[1])->name);	// ID
	currAttr->scope = strdup(currScope);						// ID == scope
	currAttr->var_func = FUNC;									// var_func

	// currScope
	currScope = strdup(root->children[1]->name);						// ID == scope

}

void m_var_decl(ast root, attr* currAttr, char* currScope){
	// tipo_esp ID ; | tipo_esp ID [NUM];
	printf("2\n");
	
	currAttr->type = strdup(nextTerminal(root->children[0])->name);	// Tipos especificador
	currAttr->name = strdup(nextTerminal(root->children[1])->name);	//ID
	currAttr->scope = strdup(currScope);						// ID == scope
	currAttr->var_func = VAR;									// var_func
}

void m_varf_decl(ast root, attr* currAttr, char* currScope){
	// tipo_esp ID | tipo_esp ID []
	printf("3\n");
	currAttr->type = strdup(nextTerminal(root->children[0])->name);	// Tipos especificador
	currAttr->name = strdup(nextTerminal(root->children[1])->name);	//ID
	currAttr->scope = strdup(currScope);						// ID == scope
	currAttr->var_func = VAR;									// var_func
}

void m_fun_act(ast root, attr* currAttr, char* currScope){
	// ID (args)
	printf("4\n");
	currAttr->type = NULL;										// Tipo
	currAttr->name = strdup(nextTerminal(root->children[0])->name);	// ID
//	strdup(currAttr->scope, currScope);	TODO: Check if need
	currAttr->scope = NULL;										// Escopo
	currAttr->var_func = FUNC;									// var_func
}

void m_var(ast root, attr* currAttr, char* currScope){
	// ID | ID[exp]	//TODO: differentiate array
	printf("5\n");
	currAttr->type = NULL;
	currAttr->name = strdup(nextTerminal(root->children[0])->name);
	currAttr->scope = strdup(currScope);
	currAttr->var_func = VAR;

}


ast nextTerminal(ast no){
	int i = 0;
	ast retorno;

	if (no==NULL){
		printf(ERR_SEM_0);
		return NULL;
	}
	
	else if(no->tok == terminal) {printf("no %s\n", no->name);return no;}
	
	while(i < no->n_child){
		retorno = nextTerminal(no->children[i++]);
		if(retorno != NULL)
			if(retorno->tok == terminal)return retorno;
	}

	return NULL;
}

