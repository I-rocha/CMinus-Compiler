#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "semantic.h"
#include "../lexical/symtab.h"

#include "../ast/ast.h"

#define MAINF "main"
#define INPUTF "input"
#define OUTPUTF "output"

// Symbol table
symTable *headEnv, *env;

// Private Func
static void semanticStart();
static void handleItem(astNo* root);
static Token handleTable(astNo* root);
static Token handleOp(astNo* no);
static int checkMain();
static void addIO();

symEntry* declared(astNo* no, symTable* target_env);
void param(astNo* no, symEntry* target);
Token lrtype(Token t1, Token t2);

void printErr(symEntry* entry, int line);

/*	Definition	*/

int checkMain(){
	if(symTLook(headEnv, MAINF))
		return 1;

	printf("Erro Semantico - Funcao main não definida\n");
	return 0;
}

void addIO(){
	symEntry* entry;

	// Input
	symTPut(headEnv,
		FUN_K,
		INPUTF,
		INT_K,
		0,
		0,
		_DECLARATION);

	// Output
	entry = symTPut(headEnv,
		FUN_K,
		OUTPUTF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);
	return;
}


void semanticStart(){
	headEnv = symTInit();
	env = headEnv;
}

symEntry* declared(astNo* no, symTable* target_env){
	symEntry* entry;
	entry = symTLook(target_env, no->instance);

	if(!entry){
		// error();
		printf("ERROR: Not declared\n");
	}

	return entry;
}

void multiple_declaration(symEntry* entry){
	if(!entry)
		return;

	if(symTGetNDef(entry) > 1){
		printErr(entry, entry->def[entry->ndef-1]);
	}
	return;
}

void param(astNo* no, symEntry* target){
	astNo* aux;
	int n_no = 0;	// Number params of function call in no

	// Aux receives child[0]
	aux = (no->len_child > 0)? no->child[0] : NULL;

	// It's supossed to be CALL_K
	if(no->label != CALL_K)
		return;
	
	// Calculate n_no 
	while(aux){
		aux = aux->sibling;
		n_no++;
	}

	// Compare No params
	if(symTGetNParam(target) != n_no){
		//	ERROR	
		printf("Semantic error: Different number of params\n");
	}


	return;
}

void handleItem(astNo* root){
	astNo *aux, *no_param;
	symEntry* entry;

	if(!root)
		return;

	// Definitions
	if(root->label == INT_K || root->label == VOID_K){
		aux = root->child[0];

		switch(aux->label){
			case FUN_K:
				no_param = aux->child[0];

				entry = symTPut(headEnv,
					aux->label,
					aux->instance,
					root->label,
					0,
					aux->line,
					_DECLARATION);

				if(aux->len_child == 1)
					break;

				// Update params
				while(no_param){
					symTAddParam(entry, no_param->label);
					no_param = no_param->sibling;
				}
				break;

			case ARG_K:
				entry = symTPut(env,
					aux->label,
					aux->instance,
					root->label,
					0,
					aux->line,
					_DECLARATION);
				break;

			// TODO: See what to do with attr = 0 for array as param
			case ARG_ARRAY_K:
				entry = symTPut(env,
					aux->label,
					aux->instance,
					root->label,
					0,
					aux->line,
					_DECLARATION);
				break;

			case ALLOC_K:
				entry = symTPut(env,
					aux->label,
					aux->instance,
					root->label,
					0,
					aux->line,
					_DECLARATION);
				break;

			case ALLOC_ARRAY_K:
				entry = symTPut(env,
					aux->label,
					aux->instance,
					root->label,
					atoi(aux->child[0]->instance),
					aux->line,
					_DECLARATION);
				break;
			default:
				entry = NULL;
		}
		multiple_declaration(entry);
	}
	else if (root->label == VAR_K || root->label == VAR_ARRAY_K){
		entry = declared(root, env);
		if(entry){
			symTUpdateCall(entry, root->line);	
		}


	}
	else if (root->label == CALL_K){
		entry = declared(root, headEnv);
		param(root, entry);
		if(entry){
			symTUpdateCall(entry, root->line);	
		}

	}
	return;
}


Token handleTable(astNo* root){
	Token ret;

	if(!root)
		return BLANK;

	handleItem(root); 


	switch(root->label){
		case WHILE_K:
			env = symTNewEnv(env, strdup("WHILE"));	// TODO: Remove strdup- should not allocate here

			// Call for child
			for(int i = 0; i < root->len_child; i++)
				ret = handleTable(root->child[i]);

			env = symTExit(env);
			break;

		case FUN_K:
			env = symTNewEnv(env, root->instance);

			// Call for child
			for(int i = 0; i < root->len_child; i++)
				ret = handleTable(root->child[i]);

			env = symTExit(env);
			break;

		case IF_K:
			// IF env
			env = symTNewEnv(env, strdup("IF"));	// TODO: Remove strdup

			// Call for child
			ret = handleTable(root->child[0]);
			ret = handleTable(root->child[1]);

			env = symTExit(env);

			// ELSE env
			if(root->len_child == 3){
				env = symTNewEnv(env, strdup("ELSE"));	// TODO: Remove strdup

				// Call for child
				ret = handleTable(root->child[2]);
				env = symTExit(env);
			}

			break;
		default:
			// Operations
			if(
			root->label == LEQ_K ||
			root->label == LESS_K ||
			root->label == GRAND_K ||
			root->label == GEQ_K ||
			root->label == EQ_K ||
			root->label == DIFF_K ||
			root->label == PLUS_K ||
			root->label == MINUS_K ||
			root->label == MULT_K ||
			root->label == DIV_K ||
			root->label == ASSIGN_K 
			){
				Token lval, rval;
				lval = handleOp(root->child[0]);
				rval = handleOp(root->child[1]);

				// Check types
				ret = lrtype(lval, rval);

			}else{
			for(int i = 0; i < root->len_child; i++)
				ret = handleTable(root->child[i]);
			}
	}
	
	handleTable(root->sibling);
	return ret;
}

Token handleOp(astNo* no){
	symEntry* ret;
	Token lval, rval;

	if(no->label == CALL_K || no->label == VAR_K || no->label == VAR_ARRAY_K){
		ret = symTLook(env, no->instance);
		return ret->type;
	}
	else if(no->label == NUM_K){
		return INT_K;
	}
	lval = handleTable(no->child[0]);
	rval = handleTable(no->child[1]);

	return lrtype(lval, rval);
}

void semantic(astNo* root){
	semanticStart();
	addIO();
	handleTable(root);
	checkMain();
}

Token lrtype(Token t1, Token t2){
	if(t1 != t2){
		printf("ERROR: Tentando realizar operacao com tipos diferentes de dados\n");
		return INT_K;
	}
	return t1;
}


void printErr(symEntry* entry, int line){
	printf("Erro semantico\n %d| %s : Multiple declaration of %s. First defined on line %d\n",line, entry->lexeme, entry->lexeme, entry->def[0]);

	printf("\n");
}
