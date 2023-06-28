#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "../cgen/cgen.h"
#include "mnemonic.h"
#include "../utils.h"

#define MEM_SZ 1000
#define DEF_ID 0 
#define DEF_STR 1 

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
}definitionByID;

typedef struct{
	char* str;
	int line;
	int* addr;
}definitionByStr;

typedef struct ListDefinition{
	union{
		definitionByID *itemId;
		definitionByStr *itemStr;
	};
	int len;
	int type;
}listDefinition;

typedef struct{
	quad** code;
	int len;
}quadList;

/* Lists operations */
void* ldGet(listDefinition* l, void* def);
int ldAdd(listDefinition* l, void* def, int line, int* addr);
int ldRm(listDefinition* l, void* def);

void printList(listDefinition* l);

/* to assembly */
void processGlobal(quad *head);
void saveReturn();
void saveBinding();
void processFunctionRec(quad* fun, listString* ls);
void processFunction(quad* fun);
void loadTemps();
void locateTemps();


static listDefinition labels, requests, calls;	// requests to labels
static stack* params;

void envInitGlobal(){
	initGlobal();
	labels.itemId = NULL;
	labels.len = 0;
	labels.type = DEF_ID;

	requests.itemId = NULL;
	requests.len = 0;
	requests.type = DEF_ID;

	calls.itemStr = NULL;
	calls.len = 0;
	calls.type = DEF_STR;

	params = NULL;
}

void endEnv(){

	// freeListDefinition();
	freeNull((void**)&labels.itemId);
	freeNull((void**)&requests.itemId);
}
void printList(listDefinition* l){
	if(!l)
		return;
	printf("List: \n");
	for(int i = 0; i < l->len; i++)
		printf("Line %d -- label %d", l->itemId[i].line, l->itemId[i].id);
	printf("\n");
}

void* ldGet(listDefinition* l, void* def){
	definitionByID* itID;
	definitionByStr* itStr;

	if(!l)
		return NULL;

	// Look each item and see if definition exists
	for(int i = 0; i < l->len; i++){
		// Type of union
		if(l->type == DEF_ID){
			itID = &l->itemId[i];		// get item
			if(itID->id == *(int*)def)	// check definition
				return itID;
		}
		// Type of union
		else if(l->type == DEF_STR){
			itStr = &l->itemStr[i];	// get item
			if(strcmp(itStr->str, (char*)def) == 0)	// check definition
				return itStr;
		}
	}
	return NULL;
}

/* Add definition to list if element not exist*/
int ldAdd(listDefinition* l, void* def, int line, int* addr){

	if(!l){
		printf("List not exist (addList)\n");
		exit(0);
		return -1;
	}
	if(ldGet(l, def))
		return 0;

	// Check type and alloc new item
	switch(l->type){
	case DEF_ID:
		l->itemId = (definitionByID*)realloc(l->itemId, sizeof(definitionByID) * (l->len + 1));
		allocateValidator((void**)&l->itemId, REALLOC_VALIDATE);
		l->itemId[l->len].id = *(int*)def;
		l->itemId[l->len].line = line;
		l->itemId[l->len].addr = addr;
		l->len++;
		return 1;
		break;
	case DEF_STR:
		l->itemStr = (definitionByStr*)realloc(l->itemStr, sizeof(definitionByStr) * (l->len + 1));
		allocateValidator((void**)&l->itemStr, REALLOC_VALIDATE);
		l->itemStr[l->len].str = strdup((char*)def);
		l->itemStr[l->len].line = line;
		l->itemStr[l->len].addr = addr;
		l->len++;
		return 1;
		break;
	default:
		return -2;
	}
	return 1;
}

/* 1: Removed
 * 0: Definition not exist
 * -1: Error
 */
int ldRm(listDefinition* l, void* def){
	definitionByID* replace_id_t, *lastItemID;
	definitionByStr* replace_str_t, *lastItemStr;
	char* str_ref_d;
	int* addr_ref_d;

	void* toRemove;

	if(!l || l->len == 0)
		return 0;

	toRemove = ldGet(l, def);
	if(!toRemove)
		return 0;

	// Check type, copy last item into replace item and free old allocated types (fields of replaced item)
	switch(l->type){
	case DEF_ID:
		lastItemID = &l->itemId[l->len-1];
		replace_id_t = (definitionByID*)toRemove;
		addr_ref_d = replace_id_t->addr;	// Saving to free later

		// Copy
		replace_id_t->id = lastItemID->id;
		replace_id_t->line = lastItemID->line;
		replace_id_t->addr = lastItemID->addr;

		// Resizing array
		l->itemId = (definitionByID*)realloc(l->itemId, sizeof(definitionByID) * (--l->len));

		if(l->len == 0){
			free(l->itemId);
			l->itemId = NULL;
		}

		free(addr_ref_d);
		break;

	case DEF_STR:
		lastItemStr = &l->itemStr[l->len-1];
		replace_str_t = (definitionByStr*)toRemove;
		str_ref_d = replace_str_t->str;	// Saving to free later

		// Copy
		replace_str_t->str = lastItemStr->str;
		replace_str_t->line = lastItemStr->line;
		replace_str_t->addr = lastItemStr->addr;

		// Resizing array
		l->itemStr = (definitionByStr*)realloc(l->itemStr, sizeof(definitionByStr) * (--l->len));

		if(l->len == 0){
			free(l->itemStr);
			l->itemStr = NULL;
		}

		free(str_ref_d);
		break;

	default:
		return 0;
	}

	return 1;
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
		newInstruction(mv, oa, arg2, 0);
		newInstruction(op, arg2, r, 0, 0);
		newInstruction(op, arg1, arg2);
		newInstruction(mv, arg2, oa, 0);
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

void stackParam(int len){
	int param;
	param = popStack(&params);
	for(int i = 0; i < len; i++){
		newInstruction(sw, sp, param, (-i-1));
	}
}

/* Conver CI to assembly */
void processFunctionRec(quad* fun, listString* ls){
	int key, positional, reg, label;

	if(!fun)
		return;

	int arg1, arg2;
	instruction* instr;

	switch(fun->op){
	case FUN_C:
		/**/
		break;
	case ARG_C:
		addListString(ls, fun->arg2);
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
		key = getKeyListString(ls, fun->arg2);
		positional = key + 1;
		if(key < 0){
			printf("## Error finding name in list (LOAD_C)\n");
			printf("## Assign 0 to positional\n");
		}
		reg = atoi(&fun->arg1[2]);
		newInstruction(mv, reg, fp, 0);
		newInstruction(lw, reg, 0, positional);
		break;
	case IFF_C:
		// Trocar a condicao
		instr = newInstruction(bc, 0, 0, -1);	// TODO: bc Must be bcn (branch conditional negate)
		label = atoi(&fun->arg2[1]);
		ldAdd(&requests, &label, getLine(), &instr->desl);
		break;
	case LABEL_C:
		label = atoi(&fun->arg1[1]);
		ldAdd(&labels, &label, getLine(), NULL);
		break;
	case GOTO_C:
		instr = newInstruction(branch, 0, 0, -1);
		label = atoi(&fun->arg1[1]);
		ldAdd(&requests, &label, getLine(), &instr->desl);
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
		key = getKeyListString(ls, fun->arg1);
		positional = key + 1;
		if(key < 0){
			printf("## Error finding name in list (STORE_C)\n");
			printf("## Assign 0 to positional\n");
		}
		reg = atoi(&fun->arg2[2]);
		newInstruction(sw, fp, reg, positional);
		break;
	case PARAM_C:
		params = addStack(params, atoi(&fun->arg1[1]));
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
		// Locate temps
		locateTemps();

		// Update next args
		stackParam(atoi(fun->result));

		instr = newInstruction(bal,0, 0, -1);

		// saving call to function to addr later
		ldAdd(&calls, (void*)fun->result, getLine(), &instr->desl);
		
		// armazenar retorno no resgitrador do CALL_C
		newInstruction(mv, atoi(&fun->arg1[1]), rd, 0); 

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