#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ast/ast.h"
#include "regbank.h"
#include "cgen.h"
#include "../lexical/symtab.h"
#include "../GLOBALS.h"


#define REGT 0
#define LITT 1

typedef struct Exp exp;

struct Exp{
	int type; // REGT or LITT
	int value;
	exp* desl;	// Only for array
};
int flabel = 0;
int labelid = 0;
extern int yylineno;
extern symTable* headEnv;

static void CGenInit();
static void cgen(quad **code, astNo* tree, char* lastScope, char* lastType);
static quad* addQuad(quad* code, Token op, char* arg1, char* arg2, char* result);
static exp genOp(quad **code, astNo* tree);
static void save(quad *head, FILE *fd);

void printSingle(quad* code){
	if(!code){
		printf("NULL\n");
		return;
	}

	printf("(%s, %s, %s, %s)\n", ctokenStr(code->op), code->arg1, code->arg2, code->result);

	return;
}

void printQuad(quad* code){
	if(!code)
		return;

	printf("(%s, %s, %s, %s)\n", ctokenStr(code->op), code->arg1, code->arg2, code->result);

	printQuad(code->next);
	return;
}

int saveCI(quad* head, char* path){
	FILE* fd;

	// Check existences
	if(!head || !path)
		return 0;

	// Error check
	if(!(fd = fopen(path, "w"))){
		printf("ERROR: saveCI()\n");
		return -1;
	}

	// Calls to save
	save(head, fd);

	// Close descriptor
	fclose(fd);

	return 1;
}


/* 
 * ########## PRIVATE ##########*/

void CGenInit(){
	initRegManager();
	return;
}

quad* addQuad(quad* code, Token op, char* arg1, char* arg2, char* result){
	if(!code)
		code = (quad*)malloc(sizeof(quad));
	else{
		code->next = (quad*) malloc(sizeof(quad));
		code = code->next;
	}
	
	code->op = op;
	code->arg1 = (arg1) ? strdup(arg1) : strdup("-");
	code->arg2 = (arg2) ? strdup(arg2) : strdup("-");
	code->result = (result) ? strdup(result) : strdup("-");
	return code;
}

quad* gen(astNo* astTree){
	quad *code, *head;

	CGenInit();

	// First instruction
	code = addQuad(NULL, BEGINCODE_C, NULL, NULL, NULL);

	// Store first position
	head = code; 

	cgen(&code, astTree, "Global", NULL);

	// Finals instruction
	code->next = addQuad(NULL, HALT_C, NULL, NULL, NULL);
	code->next->next = addQuad(NULL, ENDCODE_C, NULL, NULL, NULL);

	return head;
}

void cgen(quad **code, astNo* tree, char* lastScope, char* lastType){
	char slabel1[10], slabel2[10];
	char sreg[10];
	int aux;
	astNo* noAux;
	exp retop;

	if(!tree)
		return ;

	switch(tree->label){
		case FUN_K:	
			cleanFilled();
			aux = 0;
			lastScope = tree->instance;


			// Number of params
			if(tree->len_child > 1){
				noAux = tree->child[0];
				while(noAux){
					noAux = noAux->sibling;
					aux++;
				}
				sprintf(slabel1, "%d", aux);
			}

			*code = addQuad(*code, FUN_C, lastType, tree->instance, slabel1);
			flabel = labelid++;

			// It's know that len_child is at most 2 in this case (but in some case is 1 or even 0)
			for(int i = 0; i < tree->len_child; i++)
				cgen(code, tree->child[i], lastScope, lastType);


			// Maybe clean filled here 
			
			cgen(code, tree->sibling, lastScope, lastType);
			
			// Label to finish function
			sprintf(slabel1, "L%d", flabel);
			*code = addQuad(*code, LABEL_C, slabel1, NULL, NULL);
			*code = addQuad(*code, END_C, tree->instance, NULL, NULL);
			return;
			break;

		case INT_K:
			cgen(code, tree->child[0], lastScope, tokenStr(INT_K));
			cgen(code, tree->sibling, lastScope, tokenStr(INT_K));
			return;
			break;

		case VOID_K:
			cgen(code, tree->child[0], lastScope, tokenStr(VOID_K));
			cgen(code, tree->sibling, lastScope, tokenStr(VOID_K));
			return;
			break;

		case ARG_K:
			sprintf(sreg, "$t%d", linkRegister(tree->instance)); // reg

			*code = addQuad(*code, ARG_C, lastType, tree->instance, lastScope);
			*code = addQuad(*code, LOAD_C, sreg, tree->instance, NULL);
			cgen(code, tree->sibling, lastScope, lastType);
			return;
			break;
		case ARG_ARRAY_K:
			*code = addQuad(*code, ARG_ARRAY_C, lastType, tree->instance, lastScope);
			cgen(code, tree->sibling, lastScope, lastType);

			return;
			break;

		case ALLOC_K:
			*code = addQuad(*code, ALLOC_C, tree->instance, lastScope, NULL);
			cgen(code, tree->sibling, lastScope, NULL);

			return;
			break;
		case ALLOC_ARRAY_K:
			*code = addQuad(*code, ALLOC_ARRAY_C, tree->instance, lastScope, tree->child[0]->instance); // Child is supposed to exist
			cgen(code, tree->sibling, lastScope, NULL);

			return;
			break;
		case IF_K:
			cleanFilled();
			retop = genOp(code, tree->child[0]);
			(retop.type == REGT) ? sprintf(sreg, "$t%d", retop.value) : sprintf(sreg, "%d", retop.value);

			if(tree->child[0])
				if(tree->child[0]->len_child == 1)
					*code = addQuad(*code, EQ_C, NULL, sreg, "1");
		
			sprintf(slabel1, "L%d", labelid++);
			sprintf(slabel2, "L%d", labelid++);

			*code = addQuad(*code, IFF_C, sreg, slabel1, NULL);

			*code = addQuad(*code, START_IF_C, NULL, NULL, NULL);
			cgen(code, tree->child[1], lastScope, NULL);
			*code = addQuad(*code, END_IF_C, NULL, NULL, NULL);

			*code = addQuad(*code, GOTO_C, slabel2, NULL, NULL);
			*code = addQuad(*code, LABEL_C, slabel1, NULL, NULL);
			
			// CALL CHILD 2 - statement related to else
			if(tree->len_child > 2) {
				*code = addQuad(*code, START_ELSE_C, NULL, NULL, NULL);
				cleanFilled();
				cgen(code, tree->child[2], lastScope, NULL);
				*code = addQuad(*code, END_ELSE_C, NULL, NULL, NULL);

			}

			*code = addQuad(*code, LABEL_C, slabel2, NULL, NULL);

			cleanFilled();
			cgen(code, tree->sibling, lastScope, NULL);

			return;
			break;
		case WHILE_K:
			*code = addQuad(*code, START_WHILE_C, NULL, NULL, NULL);
			sprintf(slabel1, "L%d", labelid++);
			sprintf(slabel2, "L%d", labelid++);

			*code = addQuad(*code, LABEL_C, slabel1, NULL, NULL);
			cleanFilled();
			retop = genOp(code, tree->child[0]);
			(retop.type == REGT) ? sprintf(sreg, "$t%d", retop.value) : sprintf(sreg, "%d", retop.value);
		
			*code = addQuad(*code, IFF_C, sreg, slabel2, NULL);

			cgen(code, tree->child[1], lastScope, NULL);

			*code = addQuad(*code, END_WHILE_C, NULL, NULL, NULL);
			*code = addQuad(*code, GOTO_C, slabel1, NULL, NULL);

			*code = addQuad(*code, LABEL_C, slabel2, NULL, NULL);
			cleanFilled();
			cgen(code, tree->sibling, lastScope, NULL);
			return;
			break;
		case RETURN_K:
			if(tree->child){
				retop = genOp(code, tree->child[0]);
				(retop.type == REGT) ? sprintf(sreg, "$t%d", retop.value) : sprintf(sreg, "%d", retop.value);
				*code = addQuad(*code, RETURN_C, sreg, NULL, NULL);
			}
			else
				*code = addQuad(*code, RETURN_C, NULL, NULL, NULL);
			
			sprintf(slabel1, "L%d", flabel);
			*code = addQuad(*code, GOTO_C, slabel1, NULL, NULL);
			cgen(code, tree->sibling, lastScope, NULL);
			break;

		default:
			genOp(code, tree);
			return;
	}
	return;
}

exp genOp(quad **code, astNo* tree){
	int nreg, nnreg, sz;
	char sreg[10], sreg1[10], sreg2[10], v_arr[11], tsize_arr[11];
	exp lval, rval, ret, err, val;
	astNo* aux;


	err = (exp){.type = -1, .value = -2};

	if(!tree)
		return err;

	/*OP always have 2 children*/
	switch(tree->label){
		case PLUS_K:
			
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, ADD_C, sreg, sreg1, sreg2);

			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case MINUS_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, SUB_C, sreg, sreg1, sreg2);


			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case MULT_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			*code = addQuad(*code, MULT_C, sreg, sreg1, sreg2);
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case DIV_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, DIV_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case LEQ_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			/*OP*/
			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);

			*code = addQuad(*code, LE_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
		
	
		case LESS_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			/*OP*/
			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, LESS_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case GRAND_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
		
			/*OP*/
			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, GRAND_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case GEQ_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
		
			/*OP*/
			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, GE_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case EQ_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
		
			/*OP*/
			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, EQ_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case DIFF_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			/*OP*/
			nreg = linkRegister(NULL);
			sprintf(sreg, "$t%d", nreg);

			*code = addQuad(*code, DIFF_C, sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
			break;
			
		case ASSIGN_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			strcpy(v_arr, tree->child[0]->instance);

			sprintf(sreg1, "$t%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, ASSIGN_C, sreg1, sreg2, NULL);

			if(tree->child[0]->label == VAR_K)
				*code = addQuad(*code, STORE_C, v_arr, sreg1, NULL);


			else{ // VAR_ARRAY_K
				(lval.desl->type == REGT) ? sprintf(tsize_arr, "$t%d", lval.desl->value) : sprintf(tsize_arr, "%d", lval.desl->value);
				*code = addQuad(*code, STORE_C, v_arr, sreg1, tsize_arr);
			}
			ret = (exp){.type = LITT, .value = 1};

			break;


		case NUM_K:
			ret = (exp){.type = LITT, .value = atoi(tree->instance)};
			break;
		
		case VAR_K:
			nnreg = getRegister(tree->instance);

			if(nnreg < 0){
				nreg = linkRegister(tree->instance);
				sprintf(sreg, "$t%d", nreg);
				*code = addQuad(*code, LOAD_C, sreg, tree->instance, NULL);
			}
			else
				nreg = nnreg;

			ret = (exp){.type = REGT, .value = nreg};

			return ret;
			break;
		case VAR_ARRAY_K:
			val = genOp(code, tree->child[0]);

			// Index reg or lit
			(val.type == REGT) ? sprintf(sreg1, "$t%d", val.value) : sprintf(sreg1, "%d", val.value);

			nnreg = -1;

			// If index is lit, look for reg binded
			if(val.type == LITT){
				nnreg = getReg_(tree->instance, 1, val.value);
			}

			if(nnreg < 0){
				// Bind new reg
				nreg = (val.type == REGT)? linkRegister(NULL) : linkReg_(tree->instance, 1, val.value);
				sprintf(sreg, "$t%d", nreg);
				*code = addQuad(*code, LOAD_C, sreg, tree->instance, sreg1);
			}
			else
				nreg = nnreg;	// Reg binded

			ret.desl = (exp*)malloc(sizeof(exp));
			ret.type = REGT;
			ret.value = nreg;
			*(ret.desl) = val;
			return ret;
			break;
		case CALL_K:
			sz = 0;

			aux = (tree->child) ? tree->child[0] : NULL;

			// Look each child
			while(aux){
				val = genOp(code, aux);
				(val.type == REGT) ? sprintf(sreg, "$t%d", val.value) : sprintf(sreg, "%d", val.value);
				*code = addQuad(*code, PARAM_C, sreg, NULL, NULL);

				sz++;
				aux = aux->sibling;
			}

			nreg = getRa();
			sprintf(sreg, "$t%d", nreg);
			sprintf(sreg1, "%d", sz);
			*code = addQuad(*code, CALL_C, sreg, tree->instance, sreg1);
			
			if(!(strcmp(tree->instance, INPUTF) == 0) && !(strcmp(tree->instance, OUTPUTF) == 0))
				cleanFilled();

			ret = (exp){.type = REGT, .value = nreg};
			break;
		
		default:
			err.value = -1;
			return err;
	}

	cgen(code, tree->sibling, NULL, NULL);
	return ret;

}

void save(quad* code, FILE* fd){

	// Check NULL
	if(!code)
		return;	

	// Write to file
	fprintf(fd, "(%s, %s, %s, %s)\n", ctokenStr(code->op), code->arg1, code->arg2, code->result);

	// Calls next
	save(code->next, fd);
	return;
}

char* ctokenStr(CToken tok){
	switch(tok){
	case BLANK_C:
		return "BLANK";
		break;
	case FUN_C:
		return "FUN";
		break;
	case ARG_C:
		return "ARG";
		break;
	case ARG_ARRAY_C:
		return "ARG_ARRAY";
		break;
	case ALLOC_C:
		return "ALLOC";
		break;
	case ALLOC_ARRAY_C:
		return "ALLOC_ARRAY";
		break;
	case BEGINCODE_C:
		return "BEGINCODE";
		break;
	case HALT_C:
		return "HALT";
		break;
	case ENDCODE_C:
		return "ENDCODE";
		break;
	case END_C:
		return "END";
		break;
	case LOAD_C:
		return "LOAD";
		break;
	case IFF_C:
		return "IFF";
		break;
	case LABEL_C:
		return "LABEL";
		break;
	case GOTO_C:
		return "GOTO";
		break;
	case ADD_C:
		return "ADD";
		break;
	case SUB_C:
		return "SUB";
		break;
	case MULT_C:
		return "MULT";
		break;
	case DIV_C:
		return "DIV";
		break;
	case LE_C:
		return "LE";
		break;
	case LESS_C:
		return "LESS";
		break;
	case GRAND_C:
		return "GRAND";
		break;
	case GE_C:
		return "GE";
		break;
	case EQ_C:
		return "EQ";
		break;
	case DIFF_C:
		return "DIFF";
		break;
	case ASSIGN_C:
		return "ASSIGN";
		break;
	case STORE_C:
		return "STORE";
		break;
	case PARAM_C:
		return "PARAM";
		break;
	case CALL_C:
		return "CALL";
		break;
	case START_IF_C:
		return "START_IF";
		break;
	case END_IF_C:
		return "END_IF";
		break;
	case START_WHILE_C:
		return "START_WHILE";
		break;
	case END_WHILE_C:
		return "END_WHILE";
		break;
	case START_ELSE_C:
		return "START_ELSE";
		break;
	case END_ELSE_C:
		return "END_ELSE";
		break;
	case RETURN_C:
		return "RETURN";
		break;
	case NONE_C:
		return "-";
		break;
	default:
		return "UNKNOWN";
	}
	return "";
}

/*
int main(){
	quad *code, *curr;
	code = addQuad(NULL, "BEGIN", NULL, NULL, NULL);
	curr = code;
	printf("p: %p\n", curr);
	curr = addQuad(curr, "ADD", "$1", "$3", "$5");
	printf("p: %p\n", curr);
	curr = addQuad(curr, "SUB", "$2", "$4", "$6");
	printf("p: %p\n", curr);
	printQuad(code);

}*/

