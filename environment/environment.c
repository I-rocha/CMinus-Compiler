#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cgen/cgen.h"
#include "../utils.h"
#include "ld.h"
#include "mnemonic.h"
#include "../GLOBALS.h"
#include "environment.h"


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


// Mark definitions that needs to be adjusted (as addresses of instructions/variables/labels)
static listDefinition labels, labels_request, calls, calls_request;
static listVar* globals;	// Global var position in-memmory
static dictVar* dict_lvar;
static stack* params;
static memmory* ram;
static int regs[BIT_ARCH];

void envInitGlobal(){
	initGlobal();
	labels.itemId = NULL;
	labels.len = 0;
	labels.type = DEF_ID;

	labels_request.itemId = NULL;
	labels_request.len = 0;
	labels_request.type = DEF_ID;

	calls.itemStr = NULL;
	calls.len = 0;
	calls.type = DEF_STR;

	calls_request.itemStr = NULL;
	calls_request.len = 0;
	calls_request.type = DEF_STR;

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
	newInstruction(ram, addi, sp, -1);
	newInstruction(ram, sw, sp, rj, 0);
}

void saveBinding(){
	newInstruction(ram, addi, sp, -1);
	newInstruction(ram, sw, sp, fp, 0);	
	newInstruction(ram, mv, fp, sp, 0);
}

void allocate(listVar* lv, char* str, int len){
	addListVar(lv, str, len);

	// Var
	if(len == 0){
		newInstruction(ram, addi, sp, -1);
		return;
	}

	// Array
	newInstruction(ram, addi, sp, -len);
	return;
}

void stackParam(int len){
	const int const_desl = 2;
	int n_param;
	char* param;
	while(len > 0){
		param = popStack(&params);
		n_param = getN(param);
		if(isReg(param)){
			newInstruction(ram, sw, sp, n_param, -(len + const_desl));
		}
		else{
			newInstruction(ram, mvi, oa, n_param);
			newInstruction(ram, sw, sp, oa, -(len + const_desl));
		}
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
			newInstruction(ram, mv, reg, fp, 0);
			newInstruction(ram, addi, reg, -(key+1));
		}
		else{
			newInstruction(ram, mv, reg, fp, 0);
			newInstruction(ram, lw, reg, 0, -(key+1));
		}
		return;
	}
	key = getKeyListVar(lv, str_aux);
	if(key >= 0){
		// Reference definition
		newInstruction(ram, mv, reg, fp, 0);
		newInstruction(ram, lw, reg, 0, -(key + 1));
		return;
	}

	key = getKeyListVar(globals, var);
	if(key >= 0){
		// Global definition
		if(isArray(globals, var)){
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			newInstruction(ram, addi, reg, -(key+1));
		}
		else{
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			newInstruction(ram, lw, reg, 0, -(key+1));	
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
			newInstruction(ram, mv, reg, fp, 0);
			newInstruction(ram, sub, reg, arr_desl, 0, 0);
			newInstruction(ram, lw, reg, 0, -(key + 1));
		}
		else{
			// Desl is immediate
			newInstruction(ram, mv, reg, fp, 0);
			newInstruction(ram, lw, reg, 0, -(key + 1 + arr_desl));
		}
		return;
	}

	key = getKeyListVar(lv, str_aux);

	if(key >= 0){
		// Reference definition
		if(is_reg){
			// Desl is register
			newInstruction(ram, mv, reg, fp, 0);
			newInstruction(ram, lw, reg, 0, -(key + 1));
			newInstruction(ram, sub, reg, arr_desl, 0, 0);
			newInstruction(ram, lw, reg, 0, 0);
		}
		else{
			// Desl is immediate
			newInstruction(ram, mv, reg, fp, 0);
			newInstruction(ram, lw, reg, 0, -(key + 1 + arr_desl));
		}
		return;
	}

	key = getKeyListVar(globals, var);

	if(key >= 0){
		// Global definition
		if(is_reg){
			// Desl is register
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			newInstruction(ram, sub, reg, arr_desl, 0, 0);
			newInstruction(ram, lw, reg, 0, -(key+1));
		}
		else{
			// Desl is immediate
			newInstruction(ram, ldown, reg, (MEM_SZ-1));
			newInstruction(ram, lup, reg, (MEM_SZ-1));
			newInstruction(ram, lw, reg, 0, -(key + 1 + arr_desl));
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
			newInstruction(ram, mv, oa, fp, 0);
			newInstruction(ram, sub, oa, arr_desl, 0, 0);
			newInstruction(ram, sw, oa, reg, -(key+1));
			return;
		}
		else{
			// Desl is immediate
			newInstruction(ram, sw, fp, reg, -(key + 1 + arr_desl));
			return;
		}
	}

	key = getKeyListVar(lv, str_aux);

	if(key >= 0){
		// Save to ref

		if(is_reg){
			// Desl is register
			newInstruction(ram, mv, oa, fp, 0);
			newInstruction(ram, lw, oa, 0, -(key + 1));
			newInstruction(ram, sub, oa, arr_desl, 0, 0);
			newInstruction(ram, sw, oa, reg, 0);
			return;
		}
		else{
			// Desl is immediate
			newInstruction(ram, mv, oa, fp, 0);
			newInstruction(ram, lw, oa, 0, -(key + 1));
			newInstruction(ram, sw, oa, reg, -arr_desl);
			return;
		}
	}

	key = getKeyListVar(globals, fun->arg1);
	if(key >= 0){
		// Save to global
		if(is_reg){
			// Desl is reg
			newInstruction(ram, ldown, oa, (MEM_SZ-1));
			newInstruction(ram, lup, oa, (MEM_SZ-1));
			newInstruction(ram, sub, oa, arr_desl, 0, 0);
			newInstruction(ram, sw, oa, reg, -(key + 1));
			return;
		}
		else{
			// Desl is immediate
			newInstruction(ram, ldown, oa, (MEM_SZ-1));
			newInstruction(ram, lup, oa, (MEM_SZ-1));
			newInstruction(ram, sw, oa, reg, -(key + 1 + arr_desl));
			return;
		}
	}

	printf("Variable definition not found local or global. (%s)\n", __func__);
	return;
}

void start_decl(int** var_nested, int* deep){

	*var_nested = (int*)realloc(*var_nested, sizeof(int) * (*deep + 1));
	(*deep)++;
	*var_nested[(*deep)-1] = 0;
	
	return;
}

void end_decl(int** var_nested, int* deep){
	int nested_len;
	if(*deep <= 0)
		return;
	if(*var_nested == NULL)
		return;

	nested_len = *var_nested[*deep-1];
	for(int i = 0; i < nested_len; i++){
		newInstruction(ram, addi, sp, 1);
	}

	if(*deep == 1)
		freeNull((void**)var_nested);
	
	else{
		*var_nested = (int*)realloc(*var_nested, sizeof(int) * (*deep - 1));
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
	int desl = 0;

	for(int i = 0; i < BIT_ARCH; i++){
		if(regs[i] == 1){
			newInstruction(ram, sw, sp, i, -(desl+1));
			desl++;
		}
	}
	if(desl > 0){
		newInstruction(ram, addi, sp, -desl);
	}
	return;
}

void loadTemps(){
	int desl = -1;

	for(int i = BIT_ARCH-1; i >= 0; i--){
		if(regs[i] == 1){
			desl++;
			newInstruction(ram, mv, i, sp, 0);
			newInstruction(ram, lw, i, 0, desl);
		}
	}
	if(desl >= 0){
		newInstruction(ram, addi, sp, (desl+1));
	}
	return;
}

/* Conver CI to assembly */
void processFunctionRec(quad* fun, listVar* lv, int** var_nested, int* deep){
	char str_aux[100], ref[100] = "&";
	int label;
	int arg1, arg2;
	int reg;
	instruction* instr;

	if(!fun)
		return;

	switch(fun->op){
	case FUN_C:
		ldAdd(&calls, fun->arg2, getLine(), NULL);
		saveReturn();
		saveBinding();
		// 
		break;
	case ARG_C:
		addListVar(lv, fun->arg2, 0);
		newInstruction(ram, addi, sp, -1);
		break;
	case ARG_ARRAY_C:
		strcpy(str_aux, ref);
		strcat(str_aux, fun->arg2);
		addListVar(lv, str_aux, 0);
		newInstruction(ram, addi, sp, -1);
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
		newInstruction(ram, STOP, 0, 0, 0);
		break;
	case ENDCODE_C:
		newInstruction(ram, NOP, 0, 0, 0);	// Does nothing
		return;
		break;
	case END_C:
		if(strcmp(fun->arg1, "main") == 0){
			newInstruction(ram, STOP, 0, 0, 0);
			return;
			break;
		}
		// Update register pointer and return to function

		// Restore jump_back address
		newInstruction(ram, mv, rj, fp, 0);
		newInstruction(ram, lw, rj, 0, 1);

		// Update sp
		newInstruction(ram, mv, sp, fp, 0);
		newInstruction(ram, addi, sp, 2);

		// update fp
		newInstruction(ram, lw, fp, 0, 0);

		// update pc
		newInstruction(ram, jump, rj, 0, 0);

		return;
		break;
	case LOAD_C:
		load(fun, lv);
		break;
	
	case IFF_C:
		// Trocar a condicao
		newInstruction(ram, addi, rf, 1);
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
			newInstruction(ram, mvi, oa, 0);
			newInstruction(ram, get, oa, 0, 0);
			newInstruction(ram, lw, oa, 0, 0);
			newInstruction(ram, mv, reg, oa, 0);
			break;
		}
		else if(strcmp(fun->arg2, "output") == 0){
			reg = getN(popStack(&params));
			newInstruction(ram, mvi, oa, 1);
			newInstruction(ram, sw, oa, reg, 0);
			newInstruction(ram, print, oa, 0, 0);
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
	/*case START_IF_C:
		start_decl(var_nested, deep);
		break;*//*
	case END_IF_C:
		end_decl(var_nested, deep);
		break;*/
	/*case START_ELSE_C:
		start_decl(var_nested, deep);
		break;*//*
	case END_ELSE_C:
		end_decl(var_nested, deep);
		break;
	*/
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
		newInstruction(ram, addi, sp, -1);
		break;
	case ALLOC_ARRAY_C:
		len = atoi(head->result);
		newInstruction(ram, addi, sp, -(len));
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
	newInstruction(ram, ldown, sp, MEM_SZ-1);
	newInstruction(ram, lup, sp, MEM_SZ-1);
}

void toAssembly(quad* head){
	quadList la;
	char** functions;

	setInitial();
	processGlobal(head->next);

	
	la = getFuncions(head);
	functions = (char**)malloc(sizeof(char*) * la.len);
	allocateValidator((void**)&functions, MALLOC_VALIDATE);

	for(int i = 0; i < la.len; i++){
		functions[i] = la.code[i]->arg2;
	}

	dict_lvar = initDictVar(functions, la.len);

	
	for(int i = 0; i < la.len; i++){
		processFunction(la.code[i]);
	}
	
	updateLabels();
	updateCalls();
	printMem(ram);
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

void saveAssembly(const char* path){
	saveMem(ram, path);
	return;
}

void saveAssemblyPretty(const char* path){
	saveMemPretty(ram, path);
	return;
}