#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "../cgen/cgen.h"
#include "mnemonic.h"
#include "../utils.h"

#define MEM_SZ 1000


#define ntemps 16 // Number of temporary
#define dj 31	// data jump
#define sp 30	// stack pointer
#define fp 29	// frame pointer
#define hp 28	// heap pointer ??
#define oa 27	// override aux
#define rd 26	// return data

typedef struct{
	int id;
	int line;
	int* addr;
}definition;

typedef struct ListDefinition{
	definition* item;
	int len;
}listDefinition;

typedef struct{
	quad** code;
	int len;
}quadList;

/* Lists operations */
definition* getListDefinition(listDefinition* l, int id);
int addList(listDefinition* l, int id, int line, int* addr);
definition removeList(listDefinition* l, int id);
void printList(listDefinition* l);

/* to assembly */
void processGlobal(quad *head);
void saveReturn();
void saveBinding();
void processFunctionRec(quad* fun, listString* ls);
void processFunction(quad* fun);
void loadTemps();
void locateTemps();


static listDefinition labels, requests;	// requests to labels

void envInitGlobal(){
	initGlobal();
	labels.item = NULL;
	labels.len = 0;
}

void endEnv(){

	// freeListDefinition();
	freeNull((void**)&labels.item);
	freeNull((void**)&requests.item);
}
void printList(listDefinition* l){
	if(!l)
		return;
	printf("List: \n");
	for(int i = 0; i < l->len; i++)
		printf("Line %d -- label %d", l->item[i].line, l->item[i].id);
	printf("\n");
}

definition* getListDefinition(listDefinition* l, int id){
	definition* it;
	if(!l)
		return NULL;

	for(int i = 0; i < l->len; i++){
		it = &l->item[i];
		if(it->id == id)
			return it;
	}
	return NULL;
}

/* Add definition to list if element not exist*/
int addList(listDefinition* l, int id, int line, int* addr){

	if(!l){
		printf("List not exist (addList)\n");
		exit(0);
		return -1;
	}
	
	if(!l->item){
		l->item = (definition*)malloc(sizeof(definition));
		allocateValidator((void**)&l->item, MALLOC_VALIDATE);

		l->item[0].id = id;
		l->item[0].line = line;
		l->item[0].addr = addr;
		l->len = 1;
		return 1;
	}

	if(getListDefinition(l, id))
		return 0;
	
	l->item = (definition*)realloc(l->item, sizeof(definition) * (l->len + 1));
	allocateValidator((void**)&l->item, REALLOC_VALIDATE);

	l->item[l->len].line = line;
	l->item[l->len].id = id;	
	l->item[0].addr = addr;
	l->len++;

	return 1;
}

/* Get element specified by id, remove from list and returns it */
/* NOTE: If element does'nt exist. return is unclear */
definition removeList(listDefinition* l, int id){
	definition ret, *temp;
	ret.id = -1;
	ret.line = -1;

	temp = getListDefinition(l, id);

	if(!temp)
		return ret;

	ret = *temp;
	*temp = l->item[--l->len];

	if(l->len > 0){
		l->item = (definition*)realloc(l->item, sizeof(definition)*l->len);
		allocateValidator((void**)&l->item, REALLOC_VALIDATE);
	}
	else
		freeNull((void**)&l->item);

	return ret;
}

/* Note that quadList.code must be deallocated */
quadList getFuncions(quad* head){
	quad *curr, *temp;
	quadList fun_list;
	curr = head;

	fun_list.len = 0;
	fun_list.code = NULL;

	while(curr != NULL){
		switch(curr->op){

		// Alloc every function in list
		case FUN_C:
			fun_list.code = (quad**)realloc(fun_list.code, fun_list.len + 1);
			temp = curr;

			// Alloc main at first position
			if(
				(strcmp(curr->arg2, "main") == 0) && 
				(fun_list.len > 0)
				){
				temp = fun_list.code[0];
				fun_list.code[0] = curr;
			}
				fun_list.code[fun_list.len++] = temp;
			break;
		default:
			break;
		}
		curr = curr->next;
	}
	return fun_list;
}

int isReg(char* str){
	return (str[0] == '$')? 1 : 0;
}

/* NOTE: str must be reg or literal string */
int getN(char* str){
	return (isReg(str))? atoi(&str[2]) : atoi(str);
}

void processAritmetic(quad* fun, operation_t op, operation_t opi){
	const int arg1 = getN(fun->arg1);
	const int arg2 = getN(fun->arg2);
	const int r = getN(fun->result);
	const int isreg_arg2 = isReg(fun->arg2);
	const int isreg_r = isReg(fun->result);

	// Create instruction based on where arguments are register or immediate
	if(!isreg_arg2 && !isreg_r){
		newInstruction(opi, arg1, arg2);
		newInstruction(opi, arg1, r);
	}
	else if(!isreg_arg2 && isreg_r){
		newInstruction(mvi, arg1, arg2);
		newInstruction(op, arg1, r, 0, 0);
	}
	else if(isreg_arg2 && !isreg_r){
		newInstruction(mvi, arg1, r);
		newInstruction(op, arg1, arg2, 0, 0);
	}
	else{
		newInstruction(mv, oa, arg2, 0, 0);
		newInstruction(op, arg2, r, 0, 0);
		newInstruction(op, arg1, arg2);
		newInstruction(mv, arg2, oa);
	}
}

void processRelational(quad* fun, operation_t op, operation_t opi){
	const int arg1 = getN(fun->arg1);
	const int arg2 = getN(fun->arg2);
	// const int r = getN(fun->result);
	const int isreg_arg1 = isReg(fun->arg1);
	const int isreg_arg2 = isReg(fun->arg2);

	// Create instruction based on where arguments are register or immediate
	if(!isreg_arg1 && !isreg_arg2){
		newInstruction(mvi, oa, arg1);
		newInstruction(opi, oa, arg2);
	}
	else if(isreg_arg1 && !isreg_arg2){
		newInstruction(opi, arg1, arg2);
	}
	else if(!isreg_arg1 && isreg_arg2){
		newInstruction(opi, arg1, arg2);
	}
	else{
		newInstruction(op, arg1, arg2, 0, 0);
	}
}

void saveReturn(){
	newInstruction(addi, sp, -1);
	newInstruction(sw, sp, dj, 0);
}

void saveBinding(){
	newInstruction(addi, sp, -1);
	newInstruction(sw, sp, fp, 0);	
	newInstruction(mv, fp, sp, 0);
}

void allocate(listString* ls, char* str){
	addListString(ls, str);
	newInstruction(addi, sp, -1);
}

/* Convert to assembly a whole function, including address and control */
void processFunction(quad* fun){
	listString* ls;
	ls = newListString();

	saveReturn();
	saveBinding();
	processFunctionRec(fun, ls);
	return;
}

void loadTemps(){
	int rt;
	for(int i = 0; i < ntemps; i++){
		rt = ntemps - i;
		newInstruction(mv, rt, sp, 0);
		newInstruction(lw, rt, 0, 0);
		newInstruction(addi, sp, 1);	// update sp
	}
	return;
}

void locateTemps(){
	for(int rt = 0; rt < ntemps; rt++){
		newInstruction(addi, sp, -1);	// update sp
		newInstruction(sw, sp, rt, 0);	// allocate
	}
	return;
}


/* Conver CI to assembly */
void processFunctionRec(quad* fun, listString* ls){
	int positional, reg;

	if(!fun)
		return;

	// operation_t op;
	int arg1, arg2;
	instruction* instr;

	switch(fun->op){
	case FUN_C:
		/**/
		break;
	case ARG_C:
		/**/
		break;
	case ALLOC_C:
		// Need to considerate array
		allocate(ls, fun->arg1);
		// newInstruction(addi, sp, -1);
		break;
	case BEGINCODE_C:
		newInstruction(NOP, 0, 0, 0);	// Does nothing
		break;
	case HALT_C:
		newInstruction(STOP, 0, 0, 0);
		break;
	case ENDCODE_C:
		newInstruction(NOP, 0, 0, 0);	// Does nothing
		return;
		break;
	case END_C:
		return;
		break;
	case LOAD_C:
		// 
		positional = getKeyListString(ls, fun->arg2) + 1;
		reg = atoi(&fun->arg1[1]);
		newInstruction(mv, reg, fp);
		newInstruction(lw, reg, 0, positional);
		break;
	case IFF_C:
		// Trocar a condicao
		instr = newInstruction(bc, 0, 0, -1);	// TODO: bc Must be bcn (branch conditional negate)
		addList(&requests, atoi(&fun->arg2[1]), getLine(), &instr->desl);
		break;
	case LABEL_C:
		addList(&labels, atoi(&fun->arg1[1]), getLine(), NULL);
		break;
	case GOTO_C:
		instr = newInstruction(branch, 0, 0, -1);
		addList(&requests, atoi(&fun->arg1[1]), getLine(), &instr->desl);
		break;
	case ADD_C:
		processAritmetic(fun, add, addi);
		break;
	case SUB_C:
		processAritmetic(fun, sub, subi);
		break;
	case MULT_C:
		// processAritmetic(fun, mult);
		break;
	case DIV_C:
		// processAritmetic(fun, div);
		break;
	case LE_C:
		processRelational(fun, leq, leqi);
		break;
	case LESS_C:
		processRelational(fun, less, lessi);
		break;
	case GRAND_C:
		processRelational(fun, grand, grandi);
		break;
	case GE_C:
		processRelational(fun, geq, geqi);
		break;
	case EQ_C:
		processRelational(fun, eq, eqi);
		break;
	case DIFF_C:
		processRelational(fun, neq, neqi);
		break;
	case ASSIGN_C:
		arg1 = getN(fun->arg1);
		arg2 = getN(fun->arg2);
		isReg(fun->arg2) ? (newInstruction(mv, arg1, arg2, 0)) : (newInstruction(mvi, arg1, arg2));
		break;
	case STORE_C:
		positional = getKeyListString(ls, fun->arg2) + 1;
		reg = atoi(&fun->arg1[1]);
		newInstruction(sw, fp, reg, positional);
		break;
	case PARAM_C:
		/**/
		break;
	case RETURN_C:
		// store return data and update pointers

		if(fun->arg1){
			// save return data
			newInstruction(mv, rd, atoi(&fun->arg1[1]), 0);
		}
		// update PC
		newInstruction(mv, oa, fp, 0);
		newInstruction(addi, oa, 1);
		newInstruction(jump, oa, 0, 0);

		// Update sp
		newInstruction(mv, sp, fp, 0);
		newInstruction(lw, sp, 0, 2);

		loadTemps();

		// update fp
		newInstruction(lw, fp, 0, 0);

		/**/
		break;
	case CALL_C:
		// alocar temporarios
		// obter endereco da funcao chamada
		// fazer pulo incondicional
		// armazenar retorno no resgitrador do CALL_C

		/**/
		break;
	default:
		/**/
		break;
	}
	processFunctionRec(fun->next, ls);
}


/* Makes global allocation */
void processGlobal(quad *head){
	if(
		!head || 
		(head->op != ALLOC_C)
	 ){
		return;
	}

	newInstruction(addi, sp, -1);
	processGlobal(head->next);
}

void toAssembly(quad* head){
	quadList la;
	processGlobal(head->next);

	
	la = getFuncions(head);
	
	for(int i = 0; i < la.len; i++){
		printf("function: %s\n", la.code[i]->arg2);
	}
	
	processFunction(la.code[0]);
	
	printRam();
	/*
	if(la.code){
		free(la.code);
		la.code = NULL;
	}
	*/
}