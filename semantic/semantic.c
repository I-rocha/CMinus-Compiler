#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "semantic.h"
#include "../lexical/symtab.h"

#include "../ast/ast.h"
#include "../GLOBALS.h"

int n_err = 0;

// Symbol table
symTable *headEnv, *env;

// Private Func
static void semanticStart();
static void handleItem(astNo* root);
static astNo* handleTable(astNo* root);
static astNo* handleOp(astNo* no);
static int checkMain();
static void addPreFunctions();

static symEntry* declared(astNo* no, symTable* target_env);
static void param(astNo* no, symEntry* target);
static void lrtype(astNo* t1, astNo* t2);
static void type_declaration(symEntry* entry);

// ERROR FUNCTIONS	
static void multiple_declaration_err(int line, char* err, unsigned short def_line);
static void op_type_err(char* op1, char* type1, char* op2, char* type2, int line);
static void main_err(int line);
static void param_err(int line, char* err, int n_expected, int n_giving);
static void declaration_err(int line, char* call);
static void type_declaration_err(int line, char* err, char* type);

// Error Finishing
static void semantic_err_check();

/*	Definition	*/

int checkMain(){
	if(symTLook(headEnv, MAINF))
		return 1;

	main_err(0);
	return 0;
}

void addPreFunctions(){
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


	// isDir
	entry = symTPut(headEnv,
		FUN_K,
		ISDIRF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);


	// isActive
	entry = symTPut(headEnv,
		FUN_K,
		ISACTIVEF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);


	// getByte
	entry = symTPut(headEnv,
		FUN_K,
		GETBYTEF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 2, INT_K, INT_K);


	// getNFiles
	entry = symTPut(headEnv,
		FUN_K,
		GETNFILESF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);


	// shiftLByte
	entry = symTPut(headEnv,
		FUN_K,
		SHIFTLBYTEF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);


	// shiftRByte
	entry = symTPut(headEnv,
		FUN_K,
		SHIFTRBYTEF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);


	// getAddr
	entry = symTPut(headEnv,
		FUN_K,
		GETADDRF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);


	// DisplayByte
	entry = symTPut(headEnv,
		FUN_K,
		DISPLAYBYTEF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);

	// GetSizeName
	entry = symTPut(headEnv,
		FUN_K,
		GETSIZENAMEF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);

	// GetSizePayload
	entry = symTPut(headEnv,
		FUN_K,
		GETSIZEPAYLOADF,
		INT_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 1, INT_K);
	
	// writeInstruction
	entry = symTPut(headEnv,
		FUN_K,
		WRITEINSTRUCTIONF,
		VOID_K,
		0,
		0,
		_DECLARATION);

	symTSetParam(entry, 2, INT_K, INT_K);


	// run
	entry = symTPut(headEnv,
		FUN_K,
		RUNF,
		VOID_K,
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
		declaration_err(no->line, no->instance);
	}

	return entry;
}

void multiple_declaration(symEntry* entry){
	if(!entry)
		return;

	if(symTGetNDef(entry) > 1){
		multiple_declaration_err(entry->def[0], entry->lexeme, entry->def[0]);
	}
	return;
}

void param(astNo* no, symEntry* target){
	astNo* aux;
	int n_no = 0;	// Number params of function call in no

	if(!target)
		return;
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
		param_err(target->def[0], target->lexeme, target->nparam, n_no);
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
				no_param = (aux->child) ? aux->child[0] : NULL;

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
		type_declaration(entry);
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


astNo* handleTable(astNo* root){
	astNo* ret; 
	if(!root)
		return BLANK;

	handleItem(root); 


	switch(root->label){
		case WHILE_K:
			env = symTNewEnv(env, strdup("WHILE"));	// TODO: Remove strdup- should not allocate here

			// Call for child
			for(int i = 0; i < root->len_child; i++)
				handleTable(root->child[i]);

			env = symTExit(env);
			break;

		case FUN_K:
			env = symTNewEnv(env, root->instance);

			// Call for child
			for(int i = 0; i < root->len_child; i++)
				handleTable(root->child[i]);

			env = symTExit(env);
			break;

		case IF_K:
			// IF env
			env = symTNewEnv(env, strdup("IF"));	// TODO: Remove strdup

			// Call for child
			handleTable(root->child[0]);
			handleTable(root->child[1]);

			env = symTExit(env);

			// ELSE env
			if(root->len_child == 3){
				env = symTNewEnv(env, strdup("ELSE"));	// TODO: Remove strdup

				// Call for child
				handleTable(root->child[2]);
				env = symTExit(env);
			}

			break;
		default:
			// Operations
			if(
			root->label == CALL_K ||
			root->label == VAR_K ||
			root->label == VAR_ARRAY_K ||
			root->label == NUM_K ||
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
				ret = handleOp(root);
			}else{
			for(int i = 0; i < root->len_child; i++)
				handleTable(root->child[i]);
			}
	}
	
	handleTable(root->sibling);
	return (ret == NULL) ? root : ret;
}

astNo* handleOp(astNo* no){
	astNo *lval, *rval;

	if(!no)
		return NULL;

	// Return symEntry
	if(no->label == CALL_K || no->label == VAR_K || no->label == VAR_ARRAY_K || no->label == NUM_K){
		return no;
	}

	lval = handleTable(no->child[0]);
	rval = handleTable(no->child[1]);

	lrtype(lval, rval);
	return lval;
}

void semantic(astNo* root){
	semanticStart();
	addPreFunctions();
	handleTable(root);
	checkMain();
	semantic_err_check();
}

void lrtype(astNo* t1, astNo* t2){
	symEntry pseudo1, pseudo2;
	symEntry *e1, *e2;
	int line;

	if(!t1 || !t2)
		return;

	if(t1->label == NUM_K){
		pseudo1.type = INT_K;
		pseudo1.lexeme = t1->instance;
	}
	if(t2->label == NUM_K){
		pseudo2.type = INT_K;
		pseudo2.lexeme = t2->instance;
	}
	e1 = (t1->label == NUM_K) ? &pseudo1 : symTLook(env, t1->instance);
	e2 = (t2->label == NUM_K) ? &pseudo2 : symTLook(env, t2->instance);	
	line = t1->line;

	if(!e1 || !e2)
		return;

	if(e1->type != e2->type)
		op_type_err(e1->lexeme, tokenStr(e1->type), e2->lexeme, tokenStr(e2->type), line);

	return;
}

void type_declaration(symEntry* entry){
	if(!entry)
		return;

	if(
	entry->id == ARG_K ||
	entry->id == ARG_ARRAY_K ||
	entry->id == ALLOC_K ||
	entry->id == ALLOC_ARRAY_K
	){
		if(entry->type == VOID_K)
			type_declaration_err(entry->def[0], entry->lexeme, tokenStr(entry->type));
	}
}

void COLOR_RED(){
	printf("\033[0;31m");
}
void COLOR_RESET(){
	printf("\033[0m");
}

void type_declaration_err(int line, char* err, char* type){
	n_err++;
	COLOR_RED();
	printf("Erro semantico\n");
	COLOR_RESET();
	printf("%d| (%s) : Type declaration error, trying to declare variable %s with type %s", line, err, err, type);
	printf("\n");
	printf("\n");
}

void multiple_declaration_err(int line, char* err, unsigned short def_line){
	n_err++;
	COLOR_RED();
	printf("Erro semantico\n");
	COLOR_RESET();
	printf("%d| (%s) : Multiple declaration of %s. First defined on line %d\n",line, err, err, def_line);
	printf("\n");
	printf("\n");
}
void op_type_err(char* op1, char* type1, char* op2, char* type2, int line){
	n_err++;
	COLOR_RED();
	printf("Erro semantico\n");
	COLOR_RESET();
	printf("%d| (%s) : Different types of operands. Trying to make (%s op %s) gives (%s op %s) \n", line, op1, op1, op2, type1, type2);
	printf("\n");
	printf("\n");
}
void main_err(int line){
	n_err++;
	COLOR_RED();
	printf("Erro semantico\n");
	COLOR_RESET();
	printf("%d| (%s): Main function not defined\n", line, MAINF);
	printf("\n");
	printf("\n");
}
void param_err(int line, char* err, int n_expected, int n_giving){
	n_err++;
	COLOR_RED();
	printf("Erro semantico\n");
	COLOR_RESET();
	printf("%d| (%s): Giving wrong number of arguments. Expected [%d], giving [%d]\n", line, err, n_expected, n_giving);
	printf("\n");
	printf("\n");
}
void declaration_err(int line, char* call){
	n_err++;
	COLOR_RED();
	printf("Erro semantico\n");
	COLOR_RESET();
	printf("%d| (%s): Calling name %s which is not declared", line, call, call);
	printf("\n");
	printf("\n");
}

void semantic_err_check(){
	if(n_err == 0)
		return;
	COLOR_RED();
	printf("COMPILE FAILED");
	COLOR_RESET();
	printf(" due to %d semantic errors\n", n_err);
	exit(0);
}