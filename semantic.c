//	C	//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//	My	//
#include "ast.h"
#include "semantic.h"
#include "semantic_global.h"
#include "symtab/symtab.h"

void table(ast root){
	attr aux;
	aux.type = NULL;
	aux.name = NULL;
	aux.scope = NULL;
	aux.var_func = -1;
	aux.line = -1;

	getTokens(root, &aux, GLOBAL);	
	lookType(root, NULL, GLOBAL);
	checkMain();
}


char* lookType(ast root, char* ctype, char* currScope){
	struct symbol aux_sym;
	aux_sym.prox = NULL;
	attr currAttr;
	char *l_child_t, *r_child_t, *tmp, *isNull;
	int i = 0, err = 0;

	if (root == NULL) return ctype;

	//	Scope
	else if(root->tok == kfun_decl){						// tipo_esp ID (params) composto-decl 
		m_fun_decl(root, &currAttr, &currScope);
		
		// Children with func scope
		lookType(root->children[3], ctype, currScope);
		lookType(root->children[5], ctype, currScope);	
		
		// Update scope
		currScope = strdup(GLOBAL);
	}

	//	Terminals
	else if (root->tok == terminal){
		if(root->termTok == kID){
			return getType(root->name, currScope);
		}
		else if(root->termTok == kNUM){
			return "int";
		}
	}

	// Case with type dependence
	else if(root->tok == kexp || root->tok == ksimple_exp || root->tok == ksoma_exp 
			|| root->tok == kterm){ // <left_child> <terminal> <right child>

		// Left and right children type
		l_child_t = lookType(root->children[0], ctype, currScope);
		r_child_t = lookType(root->children[2], ctype, currScope);
		// Compare both types
		return (checkType(root, l_child_t, r_child_t))? l_child_t : "empty";
	}
	
	// Remaining 
	else{
		while(i < root->n_child){
			isNull = lookType(root->children[i++], ctype, currScope);
			if(isNull != NULL) tmp = isNull;
		}
		return tmp;
	}
	return NULL;
}


void getTokens(ast root, attr* currAttr, char* currScope){
	int i = 0;
	struct symbol aux_sym;
	aux_sym.prox = NULL;

	if(root == NULL)
		return;
	
	if(root->tok == terminal) return;
	
	/*	Not terminals	*/

	/* match fun decl*/
	if(root->tok == kfun_decl){						// tipo_esp ID (params) composto-decl 
		// match
		m_fun_decl(root, currAttr, &currScope);

		//check error
		aux_sym.content = *currAttr;
		checkDeclarationFunc(root, &aux_sym);
		addNo(&aux_sym);

		/* Inside function*/;
		getTokens(root->children[3], currAttr, currScope);	// Child params	with updated scope
		getTokens(root->children[5], currAttr, currScope);	// Child composto-decl with updated scope

		currScope = strdup(GLOBAL);	// Global scope after function
		return;
	}

	/* match var_decl*/
	else if(root->tok == kvar_decl){	// tipo_esp ID ; | tipo_esp ID [NUM];
		m_var_decl(root, currAttr, &currScope);
		
		// Check error
		aux_sym.content = *currAttr;
		checkDeclarationVar(root, &aux_sym);

		// Add symbol
		addNo(&aux_sym);	
		return;
	}
	
	/* match varf_decl*/
	else if(root->tok == kparam){	// tipo_esp ID | tipo_esp ID []
		m_varf_decl(root, currAttr, &currScope);
	
		// Check error
		aux_sym.content = *currAttr;
		checkDeclarationVar(root, &aux_sym);

		// Add symbol
		addNo(&aux_sym);

		return;
	}	
	
	/* match fun_act*/
	else if(root->tok == kact){	// ID (args)
		m_fun_act(root, currAttr, &currScope);

		//check error
		aux_sym.content = *currAttr;
		checkFunc(root, &aux_sym);

		// Next tokens
		getTokens(root->children[2], currAttr, currScope);
		return;
	}
	
	/* match var*/
	else if(root->tok == kvar){	// ID | ID[exp]	
		m_var(root, currAttr, &currScope);

		// Check error
		aux_sym.content = *currAttr;
		checkVar(root, &aux_sym);

		// Next tokens
		if(root->n_child > 1) getTokens(root->children[2], currAttr, currScope);
		return;
	}

	// Run tree
	while(i < root->n_child){
		getTokens(root->children[i++], currAttr, currScope);
	}

	return;

}


void m_fun_decl(ast root, attr* currAttr, char** currScope){
	// tipo_esp ID (params) composto-decl 
	
	// Attr
	currAttr->type = strdup(nextTerminal(root->children[0])->name);	// Tipos especificador
	currAttr->name = strdup(nextTerminal(root->children[1])->name);	// ID
	currAttr->scope = strdup(*currScope);						// ID == scope
	currAttr->var_func = FUNC;									// var_func

	// currScope
	*currScope = root->children[1]->name;						// ID == scope

}

void m_var_decl(ast root, attr* currAttr, char** currScope){
	// tipo_esp ID ; | tipo_esp ID [NUM];
	
	currAttr->type = strdup(nextTerminal(root->children[0])->name);	// Tipos especificador
	currAttr->name = strdup(nextTerminal(root->children[1])->name);	//ID
	currAttr->scope = strdup(*currScope);						// ID == scope
	currAttr->var_func = VAR;									// var_func
}

void m_varf_decl(ast root, attr* currAttr, char** currScope){
	// tipo_esp ID | tipo_esp ID []
	currAttr->type = strdup(nextTerminal(root->children[0])->name);	// Tipos especificador
	currAttr->name = strdup(nextTerminal(root->children[1])->name);	//ID
	currAttr->scope = strdup(*currScope);						// ID == scope
	currAttr->var_func = VAR;									// var_func
}

void m_fun_act(ast root, attr* currAttr, char** currScope){
	// ID (args)
	currAttr->type = NULL;										// Tipo
	currAttr->name = strdup(nextTerminal(root->children[0])->name);	// ID
	currAttr->scope = strdup (*currScope);	
	currAttr->var_func = FUNC;									// var_func
}

void m_var(ast root, attr* currAttr, char** currScope){
	// ID | ID[exp]	//TODO: differentiate array
	currAttr->type = NULL;
	currAttr->name = strdup(nextTerminal(root->children[0])->name);
	currAttr->scope = strdup(*currScope);
	currAttr->var_func = VAR;

}


ast nextTerminal(ast no){
	int i = 0;
	ast retorno;

	if (no==NULL){
		printf(ERR_SEM_0);
		return NULL;
	}
	
	else if(no->tok == terminal) {return no;}
	
	while(i < no->n_child){
		retorno = nextTerminal(no->children[i++]);
		if(retorno != NULL)
			if(retorno->tok == terminal)return retorno;
	}

	return NULL;
}


int checkDeclarationFunc(ast no, symbol sfunc){
	if(exist(sfunc->content.name, sfunc->content.scope)){
		printf(ERR_SEM);
		print(no);
		printf(" ");
		printf("LINHA: %d\n", no->line);
		return 0;	
	}
	return 1;
}

int checkDeclarationVar(ast no, symbol svar){
	if(exist(svar->content.name, svar->content.scope)){
		printf(ERR_SEM);
		print(no);
		printf(" ");
		printf("LINHA: %d\n", no->line);
		return 0;	
	}
	else if(strcmp(svar->content.type,"void") == 0){
		printf(ERR_SEM);
		print(no);
		printf(" ");
		printf("LINHA: %d\n", no->line);
		return 0;
	}
	return 1;
}

int checkFunc(ast no, symbol sfunc){
	if(!exist(sfunc->content.name, sfunc->content.scope) && !exist(sfunc->content.name, GLOBAL)){
		printf(ERR_SEM);
		print(no);
		printf(" ");
		printf("LINHA: %d\n", no->line);
		return 0;
	}	
	return 1;
}

int checkVar(ast no, symbol svar){
	if(!exist(svar->content.name, svar->content.scope) && !exist(svar->content.name, GLOBAL)){
		printf(ERR_SEM);
		print(no);
		printf(" ");
		printf("LINHA: %d\n", no->line);
		return 0;
	}
	return 1;
}

int checkType(ast no, char* t1, char* t2){
	if(t1 == NULL || t2 == NULL){
		printf(H_ERR_6);
		return -1;
	}
	if(strcmp(t1,t2) != 0){
		printf(ERR_SEM);
		print(no);
		printf(" ");
		printf("LINHA: %d\n", no->line);
		return 0;
	}
	return 1;
}

int checkMain(){
	if(!exist("main", GLOBAL))
		printf("ERRO SEMANTICO: Funcao main não declarada\n");
}
