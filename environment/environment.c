#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cgen/cgen.h"
#include "../utils.h"
#include "ld.h"
#include "mnemonic.h"
#include "../GLOBALS.h"
#include "environment.h"

int stack_len = 0;
int ftime = 1;

/* List of function begin in ci */
typedef struct{
	quad** code;
	int len;
}quadList;

/* to assembly */
void processGlobal(quad *head);
void saveReturn();
void saveBinding();
void processFunctionRec(quad* fun, listVar* lv, int** var_nested, int* deep);
void processFunction(quad* fun);
void storeContext();
void loadContext();


// Mark definitions that needs to be adjusted (as addresses of instructions/variables/labels)
static listDefinition labels, labels_request, calls, calls_request;
static listVar* globals;	// Global var position in-memmory
static dictVar* dict_lvar;
static stack* params;
static memmory* ram;
static int regs[BIT_ARCH];

void envInitGlobal(){
	// Start operation format of processor
	initGlobal();

	// Store labels address
	labels.itemId = NULL;
	labels.len = 0;
	labels.type = DEF_ID;

	// Store Labels call address
	labels_request.itemId = NULL;
	labels_request.len = 0;
	labels_request.type = DEF_ID;

	// Function address
	calls.itemStr = NULL;
	calls.len = 0;
	calls.type = DEF_STR;

	// Functions call address
	calls_request.itemStr = NULL;
	calls_request.len = 0;
	calls_request.type = DEF_STR;

	// List of global variable
	globals = newListVar();
	dict_lvar = NULL;

	params = NULL;
	ram = newMem();
}

void endEnv(){

	// freeListDefinition();
	freeNull((void**)&labels.itemId);
	freeNull((void**)&labels_request.itemId);
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
			fun_list.code = (quad**)realloc(fun_list.code, (fun_list.len+1) * sizeof(quad*));
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

int isLabel(char* str){
	return (str[0] == 'L') ? 1 : 0;
}

/* NOTE: str must be reg, label or literal string */
int getN(char* str){
	if(isReg(str))
		return atoi(&str[2]);

	if(isLabel(str))
		return atoi(&str[1]);

	return atoi(str);
}

void processAritmetic(quad* fun, operation_t op, operation_t opi){
	const int arg1 = getN(fun->arg1); // target
	const int arg2 = getN(fun->arg2);
	const int r = getN(fun->result);
	const int isreg_arg2 = isReg(fun->arg2);
	const int isreg_r = isReg(fun->result);

	// Create instruction based on where arguments are register or immediate
	if(!isreg_arg2 && !isreg_r){
		// Imm op imm
		newInstruction(ram, mvi, arg1, arg2);
		newInstruction(ram, opi, arg1, r);
	}
	else if(!isreg_arg2 && isreg_r){
		// imm op reg
		newInstruction(ram, mvi, oa, arg2);		// Store imm to reg aux
		newInstruction(ram, op, oa, r, 0, 0);	// Op reg1 to reg aux
		newInstruction(ram, mv, arg1, oa, 0);	// store to target reg
	}
	else if(isreg_arg2 && !isreg_r){
		// reg op imm
		newInstruction(ram, mv, oa, arg2, 0);	// Move reg1 to reg-aux
		newInstruction(ram, opi, oa, r, 0, 0);	// Opi imm to reg-aux
		newInstruction(ram, mv, arg1, oa, 0);	// Store to target reg
	}
	else{
		//reg op reg
		newInstruction(ram, mv, oa, arg2, 0);	// Move reg1 to aux
		newInstruction(ram, op, oa, r, 0, 0);	// Op reg2 to aux
		newInstruction(ram, mv, arg1, oa, 0);	// Store to reg target
	}
}

void processRelational(quad* fun, operation_t op, operation_t opi){
	const int operand1 = getN(fun->arg2);
	const int operand2 = getN(fun->result);
	const int isreg_operand1 = isReg(fun->arg2);
	const int isreg_operand2 = isReg(fun->result);

	// Create instruction based on where arguments are register or immediate
	if(!isreg_operand1 && !isreg_operand2){
		newInstruction(ram, mvi, oa, operand1);
		newInstruction(ram, opi, oa, operand2);
	}
	else if(isreg_operand1 && !isreg_operand2){
		newInstruction(ram, opi, operand1, operand2);
	}
	else if(!isreg_operand1 && isreg_operand2){
		// change argument position and makes inverse operation of opi
		switch(opi){
		case lessi:
			opi = geqi;
			break;
		case grandi:
			opi = leqi;
			break;
		case eqi:
			opi = neqi;
			break;
		case neqi:
			opi = eqi;
			break;
		case leqi:
			opi = grandi;
			break;
		case geqi:
			opi = lessi;
			break;
		default:
			opi = UNKNOWN;
		}
		newInstruction(ram, opi, operand2, operand1);
	}
	else{
		newInstruction(ram, op, operand1, operand2, 0, 0);
	}
}

void saveReturn(){
	if(ftime == 1){
		newInstruction(ram, addi, sp, 1);
		newInstruction(ram, sw, sp, rj, 0);
	}
	else{
		newInstruction(ram, sw, sp, rj, -(stack_len+1));
	}
}

void saveBinding(){
	if(ftime == 1){
		ftime = 0;
		newInstruction(ram, addi, sp, 1);
		newInstruction(ram, sw, sp, fp$, 0);	
		newInstruction(ram, mv, fp$, sp, 0);
	}
	else{
		newInstruction(ram, sw, sp, fp$, -(stack_len));
		newInstruction(ram, mv, fp$, sp, 0);
		newInstruction(ram, subi, fp$, stack_len);		// correction of fp
		stack_len = 0;
	}
}

void allocate(listVar* lv, char* str, int len){
	addListVar(lv, str, len);

	// Var
	if(len == 0){
		newInstruction(ram, addi, sp, 1);
		return;
	}

	// Array
	newInstruction(ram, addi, sp, len);
	return;
}

void stackParam(int len){
	const int const_desl = 2;
	int n_param;
	char* param;
	int desl;
	desl = 0;
	stack_len = len;

	if (len > 0)
		newInstruction(ram, addi, sp, (len + const_desl));

	while(len > 0){
		param = popStack(&params);
		n_param = getN(param);
		if(isReg(param)){
			newInstruction(ram, sw, sp, n_param, desl);
		}
		else{
			newInstruction(ram, mvi, oa, n_param);
			newInstruction(ram, sw, sp, oa, desl);
		}
		desl--;
		len--;
	}
}

void loadVar(listVar* lv, char* var, int reg){
	int key;
	char str_aux[100] = "&";

	strcat(str_aux, var);
	key = getKeyListVar(lv, var);

	if(key >= 0){
		// Local definition
		if(isArray(lv, var)){
			newInstruction(ram, mv, reg, fp$, 0);
			newInstruction(ram, addi, reg, (key+1));
		}
		else{
			newInstruction(ram, mv, reg, fp$, 0);
			newInstruction(ram, lw, reg, 0, (key+1));
		}
		return;
	}
	key = getKeyListVar(lv, str_aux);
	if(key >= 0){
		// Reference definition
		newInstruction(ram, mv, reg, fp$, 0);
		newInstruction(ram, lw, reg, 0, (key + 1));
		return;
	}

	key = getKeyListVar(globals, var);
	if(key >= 0){
		// Global definition
		if(isArray(globals, var)){
			/*
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			*/
			newInstruction(ram, mvi, reg, (MEM_BASIS+1));
			newInstruction(ram, addi, reg, (key+1));
		}
		else{
			/*
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			*/
			newInstruction(ram, mvi, reg, (MEM_BASIS+1));
			newInstruction(ram, lw, reg, 0, (key+1));	
		}
		return;
	}
	printf("Error, variable definition not found in local or global (%s)\n", __func__);
	return;
}

void loadVarArray(quad* fun, listVar* lv, char* var, int reg){
	const int arr_desl = getN(fun->result);
	int key;
	int is_reg;
	char str_aux[100] = "&";

	is_reg = isReg(fun->result);

	strcat(str_aux, var);
	key = getKeyListVar(lv, var);

	if(key >= 0){
		// Local definition
		if(is_reg){
			// Desl is register
			newInstruction(ram, mv, reg, fp$, 0);
			newInstruction(ram, add, reg, arr_desl, 0, 0);
			newInstruction(ram, lw, reg, 0, (key + 1));
		}
		else{
			// Desl is immediate
			newInstruction(ram, mv, reg, fp$, 0);
			newInstruction(ram, lw, reg, 0, (key + 1 + arr_desl));
		}
		return;
	}

	key = getKeyListVar(lv, str_aux);

	if(key >= 0){
		// Reference definition
		if(is_reg){
			// Desl is register
			newInstruction(ram, mv, reg, fp$, 0);
			newInstruction(ram, lw, reg, 0, (key + 1));
			newInstruction(ram, add, reg, arr_desl, 0, 0);
			newInstruction(ram, lw, reg, 0, 0);
		}
		else{
			// Desl is immediate
			newInstruction(ram, mv, reg, fp$, 0);
			newInstruction(ram, lw, reg, 0, (key + 1));
			newInstruction(ram, lw, reg, 0, arr_desl);
		}
		return;
	}

	key = getKeyListVar(globals, var);

	if(key >= 0){
		// Global definition
		if(is_reg){
			// Desl is register
			/*
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			*/
			newInstruction(ram, mvi, reg, (MEM_BASIS+1));
			newInstruction(ram, add, reg, arr_desl, 0, 0);
			newInstruction(ram, lw, reg, 0, (key+1));
		}
		else{
			// Desl is immediate
			/*
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			*/
			newInstruction(ram, mvi, reg, (MEM_BASIS+1));
			newInstruction(ram, lw, reg, 0, (key + 1 + arr_desl));
		}
		return;
	}
	printf("Error, variable definition not found local or global (%s)\n", __func__);
	return;
}

void load(quad* fun, listVar* lv){
	const int reg = getN(fun->arg1);

	regs[reg] = 1;
	if(fun->result[0] == '-'){
		// Load simple variable
		loadVar(lv, fun->arg2, reg);
		return;
	}
	// Load from array
	loadVarArray(fun, lv, fun->arg2, reg);

	return;
}	

void store(quad* fun, listVar* lv){
	int key, reg, arr_desl, is_reg;
	char str_aux[100] = "&";
	strcat(str_aux, fun->arg1);

	reg = getN(fun->arg2);
	key = getKeyListVar(lv, fun->arg1);
	is_reg = (fun->result[0] != '-')? isReg(fun->result) : 0;
	arr_desl = (fun->result[0] == '-') ? 0 : getN(fun->result);

	if(key >= 0){
		// Save to local
		if(is_reg){
			// Desl is register
			newInstruction(ram, mv, oa, fp$, 0);
			newInstruction(ram, add, oa, arr_desl, 0, 0);
			newInstruction(ram, sw, oa, reg, (key+1));
			return;
		}
		else{
			// Desl is immediate
			newInstruction(ram, sw, fp$, reg, (key + 1 + arr_desl));
			return;
		}
	}

	key = getKeyListVar(lv, str_aux);

	if(key >= 0){
		// Save to ref

		if(is_reg){
			// Desl is register
			newInstruction(ram, mv, oa, fp$, 0);
			newInstruction(ram, lw, oa, 0, (key + 1));
			newInstruction(ram, add, oa, arr_desl, 0, 0);
			newInstruction(ram, sw, oa, reg, 0);
			return;
		}
		else{
			// Desl is immediate
			newInstruction(ram, mv, oa, fp$, 0);
			newInstruction(ram, lw, oa, 0, (key + 1));
			newInstruction(ram, sw, oa, reg, arr_desl);
			return;
		}
	}

	key = getKeyListVar(globals, fun->arg1);
	if(key >= 0){
		// Save to global
		if(is_reg){
			// Desl is reg
			/*
			newInstruction(ram, ldown, oa, (MEM_SZ-1));
			newInstruction(ram, lup, oa, (MEM_SZ-1));
			*/
			newInstruction(ram, mvi, oa, (MEM_BASIS+1));
			newInstruction(ram, add, oa, arr_desl, 0, 0);
			newInstruction(ram, sw, oa, reg, (key + 1));
			return;
		}
		else{
			// Desl is immediate
			/*
			newInstruction(ram, ldown, oa, (MEM_SZ-1));
			newInstruction(ram, lup, oa, (MEM_SZ-1));
			*/
			newInstruction(ram, mvi, oa, (MEM_BASIS+1));
			newInstruction(ram, sw, oa, reg, (key + 1 + arr_desl));
			return;
		}
	}

	printf("Variable definition not found local or global. (%s)\n", __func__);
	return;
}

void start_decl(int** var_nested, int* deep){

	*var_nested = (int*)realloc(*var_nested, sizeof(int) * ((*deep) + 1));
	(*deep)++;
	(*var_nested)[(*deep)-1] = 0;
	
	return;
}

void end_decl(int** var_nested, int* deep){
	int nested_len;
	if(*deep <= 0)
		return;
	if(*var_nested == NULL)
		return;

	nested_len = (*var_nested)[*deep-1];
	for(int i = 0; i < nested_len; i++){
		newInstruction(ram, addi, sp, -1);
	}
	
	if(*deep == 1)
		freeNull((void**)var_nested);
	
	else{
		*var_nested = (int*)realloc(*var_nested, sizeof(int) * ((*deep) - 1));
		allocateValidator((void**)var_nested, REALLOC_VALIDATE);
	}

	(*deep)--;
	return;
}

/* Convert to assembly a whole function, including address and control */
void processFunction(quad* fun){
	int deep = 0;
	int** var_nested;
	listVar* lv;

	var_nested = (int**)malloc(sizeof(int*));
	*var_nested = NULL;
	lv = getListVar(dict_lvar, fun->arg2);

	for(int i = 0; i < BIT_ARCH; i++)
		regs[i] = -1;

	processFunctionRec(fun, lv, var_nested, &deep);

	return;
}

void storeTemps(){
	int ntemps_used = 0;
	int desl;

	for(int i = 0; i < BIT_ARCH; i++){
		if(regs[i] == 1){
			ntemps_used++;
		}
	}

	newInstruction(ram, addi, sp, (ntemps_used + 2));

	desl = ntemps_used - 1;
	for(int i = 0; i < BIT_ARCH; i++){
		if(regs[i] == 1){
			newInstruction(ram, sw, sp, i, -(2 + desl));
			desl--;
		}
	}

	newInstruction(ram, sw, sp, ra1$, -1);
	newInstruction(ram, sw, sp, ra2$, 0);

	return;
}

void loadTemps(){
	int desl = -1;

	newInstruction(ram, mv, ra2$, sp, 0);
	newInstruction(ram, lw, ra2$, 0, -(++desl));
	newInstruction(ram, mv, ra1$, sp, 0);
	newInstruction(ram, lw, ra1$, 0, -(++desl));
	for(int i = BIT_ARCH-1; i >= 0; i--){
		if(regs[i] == 1){
			desl++;
			newInstruction(ram, mv, i, sp, 0);
			newInstruction(ram, lw, i, 0, -desl);
		}
	}
	if(desl >= 0){
		newInstruction(ram, addi, sp, -(desl+1));
	}
	
	return;
}

/* Conver CI to assembly */
void processFunctionRec(quad* fun, listVar* lv, int** var_nested, int* deep){
	char str_aux[100], ref[100] = "&";
	int label;
	int arg1, arg2;
	int reg, reg2, nlit, fp;
	instruction* instr;

	if(!fun)
		return;

	switch(fun->op){
	case FUN_C:
		ldAdd(&calls, fun->arg2, getLine(), NULL);
		stack_len = (fun->result) ? atoi(fun->result) : 0;
		saveReturn();
		saveBinding();
		// 
		break;
	case ARG_C:
		addListVar(lv, fun->arg2, 0);
		// newInstruction(ram, addi, sp, 1);
		break;
	case ARG_ARRAY_C:
		strcpy(str_aux, ref);
		strcat(str_aux, fun->arg2);
		addListVar(lv, str_aux, 0);
		// newInstruction(ram, addi, sp, 1);
		break;
	case ALLOC_C:
		allocate(lv, fun->arg1, 0);
		
		if(*deep > 0){
			*var_nested[*deep-1] += 1;
		}
		break;
	case ALLOC_ARRAY_C:
		allocate(lv, fun->arg1, atoi(fun->result));
		
		if(*deep > 0){
			*var_nested[*deep-1] += atoi(fun->result);
		}
		break;
	case BEGINCODE_C:
		newInstruction(ram, NOP, 0, 0, 0);	// Does nothing
		break;
	case HALT_C:

		// Armazena Finish flag = 1
		newInstruction(ram, mvi, 1, FINISH_FLAG_ADDR);
		newInstruction(ram, mvi, 2, 1);
		newInstruction(ram, sw, 1, 2, 0);
		
		// GERA HALT INTERRUPT
		newInstruction(ram, halt, 0, 0, 0);

		// Time to interrupt
		newInstruction(ram, NOP, 0, 0, 0);	// Does nothing
		newInstruction(ram, NOP, 0, 0, 0);	// Does nothing

		// This shouldn't be reach for any program unless SO
		newInstruction(ram, STOP, 0, 0, 0);
		break;
	case ENDCODE_C:
		newInstruction(ram, NOP, 0, 0, 0);	// Does nothing
		return;
		break;
	case END_C:
		if(strcmp(fun->arg1, "main") == 0){

			// Armazena Finish flag = 1
			newInstruction(ram, mvi, 1, FINISH_FLAG_ADDR);
			newInstruction(ram, mvi, 2, 1);
			newInstruction(ram, sw, 1, 2, 0);
			
			// GERA HALT INTERRUPT
			newInstruction(ram, halt, 0, 0, 0);

			// Time to interrupt
			newInstruction(ram, NOP, 0, 0, 0);	// Does nothing
			newInstruction(ram, NOP, 0, 0, 0);	// Does nothing

			// This shouldn't be reach for any program unless SO
			newInstruction(ram, STOP, 0, 0, 0);

			// STOP
			// newInstruction(ram, STOP, 0, 0, 0);

			// Signals the end and wait for return
			
			/*
			// Restore jump_back address
			newInstruction(ram, mv, rj, fp$, 0);
			newInstruction(ram, lw, rj, 0, -1);
			
			// update fp$
			newInstruction(ram, lw, fp$, 0, 0);

			// update pc
			newInstruction(ram, jump, rj, 0, 0);
			*/
			// newInstruction(ram, STOP, 0, 0, 0);
			return;
			break;
		}
		// Update register pointer and return to function

		// Restore jump_back address
		newInstruction(ram, mv, rj, fp$, 0);
		newInstruction(ram, lw, rj, 0, -1);

		// Move fp addr to sp
		newInstruction(ram, mv, sp, fp$, 0);

		// update fp$
		newInstruction(ram, lw, fp$, 0, 0);

		// Update sp to last frame
		newInstruction(ram, addi, sp, -2);



		// update pc
		newInstruction(ram, jump, rj, 0, 0);

		return;
		break;
	case LOAD_C:
		load(fun, lv);
		break;
	
	case IFF_C:
		newInstruction(ram, addi, rf, 1);	// Troca a condicao
		instr = newInstruction(ram, bc, 0, 0, -1);
		label = getN(fun->arg2);
		ldAdd(&labels_request, &label, getLine(), instr->desl);
		break;
	case LABEL_C:
		label = getN(fun->arg1);
		ldAdd(&labels, &label, getLine(), NULL);
		break;
	case GOTO_C:
		instr = newInstruction(ram, branch, 0, 0, -1);
		label = getN(fun->arg1);
		ldAdd(&labels_request, &label, getLine(), instr->desl);
		break;
	case ADD_C:
		processAritmetic(fun, add, addi);
		break;
	case SUB_C:
		processAritmetic(fun, sub, subi);
		break;
	case MULT_C:
		processAritmetic(fun, mult, multi);
		break;
	case DIV_C:
		processAritmetic(fun, _div, divi);
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
		isReg(fun->arg2) ? (newInstruction(ram, mv, arg1, arg2, 0)) : (newInstruction(ram, mvi, arg1, arg2));
		break;
	case STORE_C:
		store(fun, lv);	
		break;
	case PARAM_C:
		params = addStack(params, fun->arg1);
		break;
	case RETURN_C:
		// store return_data

		if(fun->arg1[0] != '-'){
			// save return data
			if(isReg(fun->arg1))
				newInstruction(ram, mv, rd, getN(fun->arg1), 0);
			else
				newInstruction(ram, mvi, rd, getN(fun->arg1));
		}
		break;
	case CALL_C:
		if(strcmp(fun->arg2, "input") == 0){
			reg = getN(fun->arg1);
			newInstruction(ram, mvi, oa, INPUT_ADDR);
			newInstruction(ram, get, oa, 0, 0);
			newInstruction(ram, lw, oa, 0, 0);
			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "output") == 0){
			reg = getN(popStack(&params));
			newInstruction(ram, mvi, oa, OUTPUT_ADDR);
			newInstruction(ram, sw, oa, reg, 0);
			newInstruction(ram, print, oa, 0, 0);
			break;
		}
		else if(strcmp(fun->arg2, "isDir") == 0){
			fp = getN(popStack(&params));
			reg = getN(fun->arg1);
			newInstruction(ram, mv, oa, fp, 0);
			newInstruction(ram, lwHD, oa, oa, 0);
			newInstruction(ram, shiftR, oa, 0, 0, 7);
			newInstruction(ram, ANDi, oa, 1);
			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "isActive") == 0){
			fp = getN(popStack(&params));
			reg = getN(fun->arg1);
			newInstruction(ram, mv, oa, fp, 0);
			newInstruction(ram, lwHD, oa, oa, 0);
			newInstruction(ram, shiftR, oa, 0, 0, 6);
			newInstruction(ram, ANDi, oa, 1);
			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "getByte") == 0){
			fp = getN(popStack(&params));		// addr
			reg = getN(popStack(&params));		// target
			reg2 = getN(fun->arg1);				// return

			newInstruction(ram, lwHD, reg, fp, 0);
			newInstruction(ram, mv, reg2, reg, 0);
			break;
		}
		else if(strcmp(fun->arg2, "getNFiles") == 0){
			fp = getN(popStack(&params));		// fp
			reg = getN(fun->arg1);
			newInstruction(ram, mv, oa, fp, 0);
			newInstruction(ram, lwHD, oa, oa, 2);
			newInstruction(ram, ANDi, oa, 255);
			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "shiftLByte") == 0){
			reg = getN(popStack(&params));		//val
			reg2 = getN(fun->arg1);
			newInstruction(ram, mv, oa, reg, 0);
			newInstruction(ram, shiftL, oa, 0, 0, 8);
			newInstruction(ram, mv, reg2, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "shiftRByte") == 0){
			reg = getN(popStack(&params));		//val
			reg2 = getN(fun->arg1);
			newInstruction(ram, mv, oa, reg, 0);
			newInstruction(ram, shiftR, oa, 0, 0, 8);
			newInstruction(ram, mv, reg2, oa);
			break;
		}
		else if(strcmp(fun->arg2, "getAddr") == 0){
			reg = getN(popStack(&params));		// addr begin
			reg2 = getN(fun->arg1);

			newInstruction(ram, mv, ra1$, reg, 0);
			newInstruction(ram, lwHD, oa, ra1$, 0); 	// hd to reg[24:36]
			newInstruction(ram, shiftL, oa, 0, 0, 8);	// shift left
			newInstruction(ram, lwHD, oa, ra1$, 1); 	// hd to reg
			newInstruction(ram, shiftL, oa, 0, 0, 8);	// shift left
			newInstruction(ram, lwHD, oa, ra1$, 2); 	// hd to reg
			newInstruction(ram, shiftL, oa, 0, 0, 8);	// shift left
			newInstruction(ram, lwHD, oa, ra1$, 3); 	// hd to reg

			newInstruction(ram, mv, reg2, reg, 0);
			break;
		}
		else if(strcmp(fun->arg2, "getSizeName") == 0){
			fp = getN(popStack(&params));		// fp
			reg = getN(fun->arg1);

			newInstruction(ram, mv, ra1$, fp, 0);
			newInstruction(ram, mvi, oa, 0);
			newInstruction(ram, lwHD, oa, ra1$, 1); 	// hd to reg[24:36]

			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "getSizePayload") == 0){
			fp = getN(popStack(&params));		// fp
			reg = getN(fun->arg1);

			newInstruction(ram, mv, ra1$, fp, 0);
			newInstruction(ram, mvi, oa, 0);
			newInstruction(ram, lwHD, oa, ra1$, 2); 	// hd to reg[24:36]
			newInstruction(ram, shiftL, oa, 0, 0, 8);
			newInstruction(ram, lwHD, oa, ra1$, 3);

			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "displayByte") == 0){
			reg = getN(popStack(&params));

			newInstruction(ram, display, reg, 0, 0);

			break;
		}
		else if(strcmp(fun->arg2, "writeInstruction") == 0){
			reg = getN(popStack(&params));		// MI addr
			reg2 = getN(popStack(&params));		// instruction

			newInstruction(ram, swMI, reg, reg2, 0);	// write here to MI
			break;
		}

		else if(strcmp(fun->arg2, "run") == 0){
			// Jump addres to specific pc
			nlit = getN(popStack(&params));
			reg = getN(popStack(&params));
			
			newInstruction(ram, mvi, oa, nlit);
			newInstruction(ram, sb, oa, 0, 0);
			newInstruction(ram, jal, reg, 0, 0);
			break;
		}
		
		else if(strcmp(fun->arg2, "setBasis") == 0){
			// new base val
			nlit = getN(popStack(&params));
			newInstruction(ram, mvi, oa, nlit);
			newInstruction(ram, sb, oa, 0, 0);
			break;
		}
		
		else if(strcmp(fun->arg2, "runChrono") == 0){
			// addr
			nlit = getN(popStack(&params));		// basis literal
			reg = getN(popStack(&params));		// Addr
			reg2 = getN(fun->arg1);

			// Addr saved to rr$
			newInstruction(ram, mv, pa$, reg, 0);

			// Store SO context
			storeContext();

			// Update basis
			newInstruction(ram, mvi, oa, nlit);
			newInstruction(ram, sb, oa, 0, 0);

			// get first data
			newInstruction(ram, mvi, oa, VISITOR_FLAG_ADDR);
			newInstruction(ram, lw, oa, 0, 0);	// First position

			newInstruction(ram, eqi, oa, 1);
			newInstruction(ram, bc, 0, 0, 2);	// IF is not first time

			newInstruction(ram, eqi, oa, 0);
			newInstruction(ram, bc, 0, 0, 62);	// Else IF 

			/*
			newInstruction(ram, eqi, oa, 2);	
			newInstruction(ram, bc, 0, 0, );	// ELSE IF
			*/

			// IF CONTENT
			loadContext();
			newInstruction(ram, branch, 0, 0, 1);

			// ELSE IF content
			newInstruction(ram, mvi, sp, 0);

			// AFTER 

			// salva oa em sp+1
			newInstruction(ram, sw, sp, oa, 1);
			
			//salva ra1 em sp+2
			newInstruction(ram, sw, sp, ra1$, 2);

			newInstruction(ram, mvi, oa, VISITOR_FLAG_ADDR);
			newInstruction(ram, mvi, ra1$, 1);
			newInstruction(ram, sw, oa, ra1$, 0);

			//load oa em sp+1
			newInstruction(ram, mv, oa, sp, 0);
			newInstruction(ram, lw, oa, 0, 1);

			//load ra1 em sp+2
			newInstruction(ram, mv, ra1$, sp, 0);
			newInstruction(ram, lw, ra1$, 0, 2);

			newInstruction(ram, jt, osa$, pa$, 0);	// goes to rr$ and back to rt$. When back, saves to rr$
			newInstruction(ram, subi, pa$, 1);		// correction of jt

			// Store Context
			storeContext();

			// Update basis
			newInstruction(ram, mvi, oa, 0);
			newInstruction(ram, sb, oa, 0, 0);

			// load SO Context
			loadContext();

			newInstruction(ram, mv, reg2, pa$, 0);
			break;
		}

		else if(strcmp(fun->arg2, "hasFinished") == 0){
			reg2 = getN(popStack(&params));
			reg = getN(fun->arg1);

			newInstruction(ram, sb, reg2, 0, 0);
			newInstruction(ram, mvi, reg, FINISH_FLAG_ADDR);
			newInstruction(ram, lw, reg, 0, 0);
			newInstruction(ram, mvi, oa, 0);
			newInstruction(ram, sb, oa, 0, 0);
			break;
		}

		        // start(basis, addr)
        // Inicia algoritmo
        else if(strcmp(fun->arg2, "start") == 0){
            reg2 = getN(popStack(&params));     // addr
			reg = getN(popStack(&params));      // basis

			newInstruction(ram, sb, reg, 0, 0);
			newInstruction(ram, mvi, 3, 0);

			newInstruction(ram, sw, 3, 3, 1);

            newInstruction(ram, sw, 3, reg2, -1);

            newInstruction(ram, sw, 3, 3, -2);
            newInstruction(ram, sw, 3, 3, -3);
            newInstruction(ram, sw, 3, 3, -4);
            newInstruction(ram, sw, 3, 3, -5);
            newInstruction(ram, sw, 3, 3, -6);
            newInstruction(ram, sw, 3, 3, -7);
            newInstruction(ram, sw, 3, 3, -8);
            newInstruction(ram, sw, 3, 3, -9);
            newInstruction(ram, sw, 3, 3, -10);
            newInstruction(ram, sw, 3, 3, -11);
            newInstruction(ram, sw, 3, 3, -12);
            newInstruction(ram, sw, 3, 3, -13);
            newInstruction(ram, sw, 3, 3, -14);
            newInstruction(ram, sw, 3, 3, -15);
            newInstruction(ram, sw, 3, 3, -16);
            newInstruction(ram, sw, 3, 3, -17);
            newInstruction(ram, sw, 3, 3, -18);
            newInstruction(ram, sw, 3, 3, -19);
            newInstruction(ram, sw, 3, 3, -20);
            newInstruction(ram, sw, 3, 3, -21);
            newInstruction(ram, sw, 3, 3, -22);
            newInstruction(ram, sw, 3, 3, -23);
            newInstruction(ram, sw, 3, 3, -24);
            newInstruction(ram, sw, 3, 3, -25);
            newInstruction(ram, sw, 3, 3, -26);
            newInstruction(ram, sw, 3, 3, -27);
            newInstruction(ram, sw, 3, 3, -28);
            newInstruction(ram, sw, 3, 3, -29);
            newInstruction(ram, sw, 3, 3, -30);
            newInstruction(ram, sw, 3, 3, -31);
			newInstruction(ram, sb, 3, 0, 0);
			break;
		}

		       // runCode(addr, basis)
        // Roda com preempção
        else if(strcmp(fun->arg2, "runCode") == 0){
            reg2 = getN(popStack(&params));     // basis
			reg = getN(popStack(&params));      // addr

			// Armazena sp
            newInstruction(ram, mvi, 1, 2);
            newInstruction(ram, sw, 1, sp, 0);

			// Armazena fp
			newInstruction(ram, mvi, 1, 0);
            newInstruction(ram, sw, 1, fp$, 0);
			
            newInstruction(ram, mv, 1, sp, 0);
            newInstruction(ram, sb, reg2, 0, 0);
            newInstruction(ram, mvi, 0, 0);

            newInstruction(ram, mvi, 0, 0);
            newInstruction(ram, mvi, 1, 0);
            newInstruction(ram, mvi, 2, 0);
            newInstruction(ram, mvi, 3, 0);
            newInstruction(ram, mvi, 4, 0);
            newInstruction(ram, mvi, 5, 0);
            newInstruction(ram, mvi, 6, 0);
            newInstruction(ram, mvi, 7, 0);
            newInstruction(ram, mvi, 8, 0);
            newInstruction(ram, mvi, 9, 0);
            newInstruction(ram, mvi, 10, 0);
            newInstruction(ram, mvi, 11, 0);
            newInstruction(ram, mvi, 12, 0);
            newInstruction(ram, mvi, 13, 0);
            newInstruction(ram, mvi, 14, 0);
            newInstruction(ram, mvi, 15, 0);
            newInstruction(ram, mvi, 16, 0);
            newInstruction(ram, mvi, 17, 0);
            newInstruction(ram, mvi, 18, 0);
            newInstruction(ram, mvi, 19, 0);
            newInstruction(ram, mvi, 20, 0);
            newInstruction(ram, mvi, 21, 0);
            newInstruction(ram, mvi, ra2$, 0);
            newInstruction(ram, mvi, ra1$, 0);
            newInstruction(ram, mvi, rd, 0);
            newInstruction(ram, mvi, oa, 0);
            newInstruction(ram, mvi, fp$, 0);
            newInstruction(ram, mvi, sp, 0);
            newInstruction(ram, mvi, rf, 0);
            newInstruction(ram, mvi, rj, 0);
            
           
            newInstruction(ram, lw, 0, 0, -2);
            newInstruction(ram, lw, 1, 0, -3);
            newInstruction(ram, lw, 2, 0, -4);
            newInstruction(ram, lw, 3, 0, -5);
            newInstruction(ram, lw, 4, 0, -6);
            newInstruction(ram, lw, 5, 0, -7);
            newInstruction(ram, lw, 6, 0, -8);
            newInstruction(ram, lw, 7, 0, -9);
            newInstruction(ram, lw, 8, 0, -10);
            newInstruction(ram, lw, 9, 0, -11);
            newInstruction(ram, lw, 10, 0, -12);
            newInstruction(ram, lw, 11, 0, -13);
            newInstruction(ram, lw, 12, 0, -14);
            newInstruction(ram, lw, 13, 0, -15);
            newInstruction(ram, lw, 14, 0, -16);
            newInstruction(ram, lw, 15, 0, -17);
            newInstruction(ram, lw, 16, 0, -18);
            newInstruction(ram, lw, 17, 0, -19);
            newInstruction(ram, lw, 18, 0, -20);
            newInstruction(ram, lw, 19, 0, -21);
            newInstruction(ram, lw, 20, 0, -22);
            newInstruction(ram, lw, 21, 0, -23);
            newInstruction(ram, lw, ra2$, 0, -24);
            newInstruction(ram, lw, ra1$, 0, -25);
            newInstruction(ram, lw, rd, 0, -26);
            newInstruction(ram, lw, oa, 0, -27);
            newInstruction(ram, lw, fp$, 0, -28);
            newInstruction(ram, lw, sp, 0, -29);
            newInstruction(ram, lw, rf, 0, -30);
            newInstruction(ram, lw, rj, 0, -31);

            newInstruction(ram, mvi, pa$, 0);
            newInstruction(ram, lw, pa$, 0, -1);

            newInstruction(ram, jt, osa$, pa$, 0);	// goes to pa$ and back to osa$. When back, saves to rr$
            newInstruction(ram, addi, pa$, -1);
            newInstruction(ram, mvi, osa$, 0);

            newInstruction(ram, sw, osa$, pa$, -1);
            newInstruction(ram, sw, osa$, 0, -2);

            newInstruction(ram, sw, osa$, 1, -3);
            newInstruction(ram, sw, osa$, 2, -4);
            newInstruction(ram, sw, osa$, 3, -5);
            newInstruction(ram, sw, osa$, 4, -6);
            newInstruction(ram, sw, osa$, 5, -7);
            newInstruction(ram, sw, osa$, 6, -8);
            newInstruction(ram, sw, osa$, 7, -9);
            newInstruction(ram, sw, osa$, 8, -10);
            newInstruction(ram, sw, osa$, 9, -11);
            newInstruction(ram, sw, osa$, 10, -12);
            newInstruction(ram, sw, osa$, 11, -13);
            newInstruction(ram, sw, osa$, 12, -14);
            newInstruction(ram, sw, osa$, 13, -15);
            newInstruction(ram, sw, osa$, 14, -16);
            newInstruction(ram, sw, osa$, 15, -17);
            newInstruction(ram, sw, osa$, 16, -18);
            newInstruction(ram, sw, osa$, 17, -19);
            newInstruction(ram, sw, osa$, 18, -20);
            newInstruction(ram, sw, osa$, 19, -21);
            newInstruction(ram, sw, osa$, 20, -22);
            newInstruction(ram, sw, osa$, 21, -23);

            newInstruction(ram, sw, osa$, ra2$, -24);
            newInstruction(ram, sw, osa$, ra1$, -25);
            newInstruction(ram, sw, osa$, rd, -26);
            newInstruction(ram, sw, osa$, oa, -27);
            newInstruction(ram, sw, osa$, fp$, -28);
            newInstruction(ram, sw, osa$, sp, -29);
            newInstruction(ram, sw, osa$, rf, -30);
            newInstruction(ram, sw, osa$, rj, -31);

            newInstruction(ram, mvi, 1, 0);
            newInstruction(ram, sb, 1, 0, 0);

			// recupera sp
            newInstruction(ram, mvi, sp, 2);
            newInstruction(ram, lw, sp, 0, 0);

			// recupera fp
			newInstruction(ram, mvi, fp$, 0);
            newInstruction(ram, lw, fp$, 0, 0);

			break;
		}

		// isFinished(basis)
        // Retorna se um algoritmo já terminou o processamento
        else if(strcmp(fun->arg2, "isFinished") == 0){
			reg = getN(popStack(&params));      // basis
            reg2 = getN(fun->arg1);				// return

			newInstruction(ram, lw, reg, 0, 1);
            newInstruction(ram, mv, reg2, reg, 0);
			break;
		}

		else if(strcmp(fun->arg2, "printProgram") == 0){
			reg = getN(popStack(&params));      // program

			newInstruction(ram, dm, 0, 0, 1);		// Clear
            newInstruction(ram, dm, 0, 0, 581);		// E
            newInstruction(ram, dm, 0, 0, 632);		// x
            newInstruction(ram, dm, 0, 0, 613);		// e
            newInstruction(ram, dm, 0, 0, 611);		// c
            newInstruction(ram, dm, 0, 0, 629);		// u
            newInstruction(ram, dm, 0, 0, 628);		// t
            newInstruction(ram, dm, 0, 0, 609);		// a
            newInstruction(ram, dm, 0, 0, 622);		// n
            newInstruction(ram, dm, 0, 0, 612);		// d
            newInstruction(ram, dm, 0, 0, 623);		// o
            newInstruction(ram, dm, 0, 0, 192);		// \n
            newInstruction(ram, dm, 0, 0, 592);		// P
            newInstruction(ram, dm, 0, 0, 626);		// r
            newInstruction(ram, dm, 0, 0, 623);		// o
			newInstruction(ram, dm, 0, 0, 615);		// g
			newInstruction(ram, dm, 0, 0, 626);		// r
			newInstruction(ram, dm, 0, 0, 609);		// a
			newInstruction(ram, dm, 0, 0, 621);		// m
			newInstruction(ram, dm, 0, 0, 609);		// a
			newInstruction(ram, dm, 0, 0, 544);		// " "

			newInstruction(ram, eqi, reg, 1);
			newInstruction(ram, bc, 0, 0, 18);
			
			newInstruction(ram, eqi, reg, 2);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 3);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 4);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 5);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 6);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 7);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 8);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 9);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, eqi, reg, 10);
			newInstruction(ram, bc, 0, 0, 18);

			newInstruction(ram, dm, 0, 0, 561);	// 1
			newInstruction(ram, branch, 0, 0, 18);
			
			newInstruction(ram, dm, 0, 0, 562);	// 2
			newInstruction(ram, branch, 0, 0, 16);

			newInstruction(ram, dm, 0, 0, 563);	// 3
			newInstruction(ram, branch, 0, 0, 14);

			newInstruction(ram, dm, 0, 0, 564);	// 4
			newInstruction(ram, branch, 0, 0, 12);

			newInstruction(ram, dm, 0, 0, 565);	// 5
			newInstruction(ram, branch, 0, 0, 10);

			newInstruction(ram, dm, 0, 0, 566);	// 6
			newInstruction(ram, branch, 0, 0, 8);

			newInstruction(ram, dm, 0, 0, 567);	// 7
			newInstruction(ram, branch, 0, 0, 6);

			newInstruction(ram, dm, 0, 0, 568);	// 8
			newInstruction(ram, branch, 0, 0, 4);

			newInstruction(ram, dm, 0, 0, 569);	// 9
			newInstruction(ram, branch, 0, 0, 2);

			newInstruction(ram, dm, 0, 0, 561);	// 1
			newInstruction(ram, dm, 0, 0, 560);	// 0
			
			break;
		}

		else if(strcmp(fun->arg2, "printQtd") == 0){
			newInstruction(ram, dm, 0, 0, 1);
            newInstruction(ram, dm, 0, 0, 593);
            newInstruction(ram, dm, 0, 0, 629);
            newInstruction(ram, dm, 0, 0, 609);
            newInstruction(ram, dm, 0, 0, 622);
            newInstruction(ram, dm, 0, 0, 628);
            newInstruction(ram, dm, 0, 0, 623);
            newInstruction(ram, dm, 0, 0, 627);
            newInstruction(ram, dm, 0, 0, 544);
            newInstruction(ram, dm, 0, 0, 609);
            newInstruction(ram, dm, 0, 0, 620);
            newInstruction(ram, dm, 0, 0, 615);
            newInstruction(ram, dm, 0, 0, 623);
            newInstruction(ram, dm, 0, 0, 627);
            newInstruction(ram, dm, 0, 0, 575);
			break;
		}

		else if(strcmp(fun->arg2, "printDigiteID") == 0){
			newInstruction(ram, dm, 0, 0, 1);
            newInstruction(ram, dm, 0, 0, 580);
            newInstruction(ram, dm, 0, 0, 617);
            newInstruction(ram, dm, 0, 0, 615);
            newInstruction(ram, dm, 0, 0, 617);
            newInstruction(ram, dm, 0, 0, 628);
            newInstruction(ram, dm, 0, 0, 613);
            newInstruction(ram, dm, 0, 0, 544);
            newInstruction(ram, dm, 0, 0, 623);
            newInstruction(ram, dm, 0, 0, 544);
            newInstruction(ram, dm, 0, 0, 585);
            newInstruction(ram, dm, 0, 0, 580);
			break;
		}

		else if(strcmp(fun->arg2, "printTurnOff") == 0){
			newInstruction(ram, dm, 0, 0, 1);
            newInstruction(ram, dm, 0, 0, 580);
            newInstruction(ram, dm, 0, 0, 613);
            newInstruction(ram, dm, 0, 0, 627);
            newInstruction(ram, dm, 0, 0, 620);
            newInstruction(ram, dm, 0, 0, 617);
            newInstruction(ram, dm, 0, 0, 615);
            newInstruction(ram, dm, 0, 0, 609);
            newInstruction(ram, dm, 0, 0, 626);
            newInstruction(ram, dm, 0, 0, 575);
            newInstruction(ram, dm, 0, 0, 192);
            newInstruction(ram, dm, 0, 0, 561);
            newInstruction(ram, dm, 0, 0, 688);
            newInstruction(ram, dm, 0, 0, 544);
            newInstruction(ram, dm, 0, 0, 595);
            newInstruction(ram, dm, 0, 0, 617);
            newInstruction(ram, dm, 0, 0, 621);
            newInstruction(ram, dm, 0, 0, 544);
            newInstruction(ram, dm, 0, 0, 562);
            newInstruction(ram, dm, 0, 0, 688);
            newInstruction(ram, dm, 0, 0, 544);
            newInstruction(ram, dm, 0, 0, 590);
            newInstruction(ram, dm, 0, 0, 609);
            newInstruction(ram, dm, 0, 0, 623);
			break;
		}

		storeTemps();
		stackParam(atoi(fun->result)); // Update next args

		// Detour
		instr = newInstruction(ram, bal,0, 0, -1);

		// saving call to function to addr later
		ldAdd(&calls_request, (void*)fun->arg2, getLine(), instr->desl);
		
		loadTemps();
		newInstruction(ram, mv, getN(fun->arg1), rd, 0); 	// Return data
		break;
	
	case START_WHILE_C:
		start_decl(var_nested, deep);
		break;
	case END_WHILE_C:
		end_decl(var_nested, deep);
		break;
	// Add start IF start ELSE
	default:
		break;
	}
	processFunctionRec(fun->next, lv, var_nested, deep);
}

void updateLabels(){
	definitionByID* lb;
	int id_req, line_req;
	int* desl_addr;

	// Get label id that matches with label_request and update address
	for(int i = 0; i < labels_request.len; i++){

		// mapping attr of itemId
		id_req = labels_request.itemId[i].id;
		desl_addr = labels_request.itemId[i].addr;
		line_req = labels_request.itemId[i].line;

		// item requested
		lb = (definitionByID*) ldGet(&labels, (void*)&id_req);
		
		if(!lb){
			printf("ERROR: Missing label L%d\n", id_req);
			return;
		}
		*desl_addr = (lb->line - line_req);
	}
	return;
}

void updateCalls(){
	definitionByStr* called;
	char* str_req;
	int line_req;
	int* desl_addr;

	for(int i = 0; i < calls_request.len; i++){
		// Mapping attr of itemStr
		str_req = calls_request.itemStr[i].str;
		desl_addr = calls_request.itemStr[i].addr;
		line_req = calls_request.itemStr[i].line;

		// Item requested
		called = (definitionByStr*) ldGet(&calls, (void*)str_req);

		if(!called){
			printf("ERROR: missing def of function %s\n", str_req);
			return;
		}
		*desl_addr = (called->line - line_req);
	}
}

/* Makes global allocation */
void processGlobal(quad *head){
	int len = 0;
	if(
		!head || 
		(head->op == FUN_C)
	 ){
		return;
	}
	switch(head->op){
	case ALLOC_C:
		newInstruction(ram, addi, sp, 1);
		break;
	case ALLOC_ARRAY_C:
		len = atoi(head->result);
		newInstruction(ram, addi, sp, (len));
		break;
	default:
		printf("Unexpected symbol at (%s)\n", __func__);
		return;
	}

	addListVar(globals, head->arg1, len);
	processGlobal(head->next);
}

void setInitial(){
	newInstruction(ram, NOP, 0, 0, 0);			// First instruction should be NOP to avoid bug's at quartus
	/*
	newInstruction(ram, ldown, sp, MEM_SZ-1);
	newInstruction(ram, lup, sp, MEM_SZ-1);
	*/

	newInstruction(ram, mvi, sp, (MEM_BASIS+1));
	newInstruction(ram, mvi, oa, FINISH_FLAG_ADDR);
	newInstruction(ram, mvi, ra1$, 0);
	newInstruction(ram, sw, oa, ra1$, 0);
}

memmory* toAssembly(quad* head){
	quadList la;
	char** functions;

	setInitial();
	processGlobal(head->next);

	
	la = getFuncions(head);
	functions = (char**)malloc(sizeof(char*) * la.len);
	allocateValidator((void**)&functions, MALLOC_VALIDATE);

	// Extract function name
	for(int i = 0; i < la.len; i++){
		functions[i] = la.code[i]->arg2;
	}

	dict_lvar = initDictVar(functions, la.len);

	
	for(int i = 0; i < la.len; i++){
		processFunction(la.code[i]);
	}
	
	updateLabels();
	updateCalls();
	
	return ram;
	/*
	freeNull((void**)&functions);
	*/
	/*
	if(la.code){
		free(la.code);
		la.code = NULL;
	}
	*/
}

void saveBin(const char* path){
	saveMem(ram, path);
	return;
}

void saveAssembly(const char* path){
	saveMemPretty(ram, path);
	return;
}

void saveBinQuartus(const char* path){
	saveMemQuartusFormact(ram, path);
	return;
}
void storeContext(){
	int desl;
	desl = 0;
	
	newInstruction(ram, addi, sp, ntemps);

	for (int reg = (ntemps - 1); reg >= 0; reg--){
		newInstruction(ram, sw, sp, reg, -(desl));
		desl++;
	}
	newInstruction(ram, addi, sp, 7);
	newInstruction(ram, sw, sp, ra2$, -6);
	newInstruction(ram, sw, sp, ra1$, -5);
	newInstruction(ram, sw, sp, rd, -4);
	newInstruction(ram, sw, sp, oa, -3);
	newInstruction(ram, sw, sp, fp$, -2);
	newInstruction(ram, sw, sp, rf, -1);
	newInstruction(ram, sw, sp, rj, 0);

	newInstruction(ram, mvi, oa, SP_ADDR);
	newInstruction(ram, sw, oa, sp, 0);		// Store sp
}

void loadContext(){
	int desl;
	desl = 0;
	
	newInstruction(ram, mvi, sp, SP_ADDR);
	newInstruction(ram, lw, sp, 0, 0);		// Load sp

	newInstruction(ram, mv, rj, sp, 0);
	newInstruction(ram, lw, rj, 0, 0);

	newInstruction(ram, mv, rf, sp, 0);
	newInstruction(ram, lw, rf, 0, -1);

	newInstruction(ram, mv, fp$, sp, 0);
	newInstruction(ram, lw, fp$, 0, -2);

	newInstruction(ram, mv, oa, sp, 0);
	newInstruction(ram, lw, oa, 0, -3);

	newInstruction(ram, mv, rd, sp, 0);
	newInstruction(ram, lw, rd, 0, -4);

	newInstruction(ram, mv, ra1$, sp, 0);
	newInstruction(ram, lw, ra1$, 0, -5);

	newInstruction(ram, mv, ra2$, sp, 0);
	newInstruction(ram, lw, ra2$, 0, -6);

	desl = 7;

	for (int reg = (ntemps - 1); reg >= 0; reg--){
		newInstruction(ram, mv, reg, sp, 0);
		newInstruction(ram, lw, reg, 0, -(desl));
		desl++;
	}

	newInstruction(ram, subi, sp, (ntemps + 7));
}