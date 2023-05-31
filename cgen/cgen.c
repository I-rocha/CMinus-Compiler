#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ast/ast.h"
#include "regbank.h"
#include "cgen.h"
#include "../lexical/symtab.h"

#define REGT 0
#define LITT 1

typedef struct{
	int type; // REGT or LITT
	int value;
}exp;

int labelid = 0;
extern int yylineno;
extern symTable* headEnv;

static void CGenInit();
static void cgen(quad **code, astNo* tree, char* lastScope, char* lastType);
static quad* addQuad(quad* code, char* op, char* arg1, char* arg2, char* result);
static exp genOp(quad **code, astNo* tree);
static void save(quad *head, FILE *fd);

void printSingle(quad* code){
	if(!code){
		printf("NULL\n");
		return;
	}

	printf("(%s, %s, %s, %s)\n", code->op, code->arg1, code->arg2, code->result);

	return;
}

void printQuad(quad* code){
	if(!code)
		return;

	printf("(%s, %s, %s, %s)\n", code->op, code->arg1, code->arg2, code->result);

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
	regBankInit();
	return;
}

quad* addQuad(quad* code, char* op, char* arg1, char* arg2, char* result){
	if(!code)
		code = (quad*)malloc(sizeof(quad));
	else{
		code->next = (quad*) malloc(sizeof(quad));
		code = code->next;
	}
	
	code->op = (op) ? strdup(op) : strdup("-");
	code->arg1 = (arg1) ? strdup(arg1) : strdup("-");
	code->arg2 = (arg2) ? strdup(arg2) : strdup("-");
	code->result = (result) ? strdup(result) : strdup("-");
	printSingle(code);
	return code;
}

quad* gen(astNo* astTree){
	quad *code, *head;

	CGenInit();

	// First instruction
	code = addQuad(NULL, "BEGINCODE", NULL, NULL, NULL);

	// Store first position
	head = code; 

	cgen(&code, astTree, "Global", NULL);

	// Finals instruction
	code->next = addQuad(NULL, "HALT", NULL, NULL, NULL);
	code->next->next = addQuad(NULL, "ENDCODE", NULL, NULL, NULL);

	return head;
}

void cgen(quad **code, astNo* tree, char* lastScope, char* lastType){
	char sreg[10];

	if(!tree)
		return ;

	switch(tree->label){
		case FUN_K:

			lastScope = tree->label;
			*code = addQuad(*code, tree->type, lastType, tree->label, NULL);

			// It's know that len_child is at most 2 in this case (but in some case is 1)
			for(int i = 0; i < tree->len_child; i++)
				cgen(code, tree->child[i], lastScope, lastType);


			/* TODO: NEED TO CHANGE THIS - ONLY WORKS FOR VARIABLES WITH THE SAME NAME BUT DIFFERENTE FUNC SCOPE*/
			/* THIS FUNC PREVENTS DIFFERENTs VARIABLEs WITH SAME NAME TO BE ASSOCIATED WITH THE SAME REG */
			cleanFilled();
			cgen(code, tree->sibling, lastScope, lastType);

			*code = addQuad(*code, "END", tree->label, NULL, NULL);

			return;
			break;

		case TYPE_K:
			cgen(code, tree->child[0], lastScope, tree->label);
			cgen(code, tree->sibling, lastScope, tree->label);
			return;
			break;

		case ARG_K:
			sprintf(sreg, "$t%d", linkReg(tree->label)); // reg

			*code = addQuad(*code, tree->type, lastType, tree->label, lastScope);
			*code = addQuad(*code, "LOAD", sreg, tree->label, NULL);
			cgen(code, tree->sibling, lastScope, lastType);
			return;
			break;

		case ALLOC_K:
			*code = addQuad(*code, tree->type, tree->label, lastScope, NULL);
			cgen(code, tree->sibling, lastScope, NULL);

			return;
		break;
	
		case IF_K:
			exp retop;
			char slabel1[10], slabel2[10];

			retop = genOp(code, tree->child[0]);
			(retop.type == REGT) ? sprintf(sreg, "$t%d", retop.value) : sprintf(sreg, "%d", retop.value);
		
			// TODO: LABEL MUST CREATE ITS OWN NAME WITHOUT CONFLICT
			sprintf(slabel1, "L%d", labelid++);
			sprintf(slabel2, "L%d", labelid++);
			*code = addQuad(*code, "IFF", sreg, slabel1, NULL);

			// LEN_CHILD is at most 3
			//TODO: IT NEEDS TO DEAL WITH THE CASE WHERE CHILD 1 SUPPOSED TO BE FIRST IFF STATEMENT DOESNT EXIST	RIGHT NOW IS WORKING ONLY IF STATEMENT RELATED TO IF IS NEVER NULL
			//CALL CHILD 1 - statement related to if
			cgen(code, tree->child[1], lastScope, NULL);

			*code = addQuad(*code, "GOTO", slabel2, NULL, NULL);

			// CALL CHILD 2 - statement related to else
			if(tree->len_child > 2) {
				*code = addQuad(*code, "LABEL", slabel1, NULL, NULL);
				cgen(code, tree->child[2], lastScope, NULL);

			}

			*code = addQuad(*code, "LABEL", slabel2, NULL, NULL);
			cgen(code, tree->sibling, lastScope, NULL);

			return;
			break;
		case WHILE_K:
			exp retop;
			char slabel1[10], slabel2[10];

			// TODO: LABEL MUST CREATE ITS OWN NAME WITHOUT CONFLICT
			sprintf(slabel1, "L%d", labelid++);
			sprintf(slabel2, "L%d", labelid++);

			*code = addQuad(*code, "LABEL", slabel1, NULL, NULL);

			retop = genOp(code, tree->child[0]);
			(retop.type == REGT) ? sprintf(sreg, "$t%d", retop.value) : sprintf(sreg, "%d", retop.value);
		

			*code = addQuad(*code, "IFF", sreg, slabel2, NULL);

			cgen(code, tree->child[1], lastScope, NULL);

			*code = addQuad(*code, "GOTO", slabel1, NULL, NULL);

			*code = addQuad(*code, "LABEL", slabel2, NULL, NULL);
			cgen(code, tree->sibling, lastScope, NULL);
			return;
			break;
		case RETURN_K:
			exp retop;
			if(tree->child){
				retop = genOp(code, tree->child[0]);
				(retop.type == REGT) ? sprintf(sreg, "$t%d", retop.value) : sprintf(sreg, "%d", retop.value);
				*code = addQuad(*code, "RETURN", sreg, NULL, NULL);
			}
			else
				*code = addQuad(*code, "RETURN", NULL, NULL, NULL);

			cgen(code, tree->sibling, lastScope, NULL);
			break;

		case CALL_K:
			genOp(code, tree);
			return;
		break;

		case ASSIGN_K:
			genOp(code, tree);
			return;
			break;

		// TODO: MAYBE CHANGE THIS
		default:
			genOp(code, tree);
			return;
	}
	return;
}

exp genOp(quad **code, astNo* tree){
	char sreg[10], sreg1[10], sreg2[10];
	exp ret, err;
	int nreg;
	exp lval, rval;

	err = (exp){.type = -1, .value = -2};

	if(!tree)
		return err;

	/*OP always have 2 children*/
	switch(tree->label){
		case PLUS_K:
			
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, "ADD", sreg, sreg1, sreg2);

			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case MINUS_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, "SUB", sreg, sreg1, sreg2);


			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case MULT_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			*code = addQuad(*code, "MULT", sreg, sreg1, sreg2);
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case DIV_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			*code = addQuad(*code, "DIV", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case LEQ_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			/*OP*/
			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);

			*code = addQuad(*code, "LE", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
		
	
		case LESS_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			/*OP*/
			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, "LESS", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case GRAND_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
		
			/*OP*/
			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, "GRAND", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case GEQ_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
		
			/*OP*/
			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, "GE", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case EQ_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
		
			/*OP*/
			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			*code = addQuad(*code, "EQ", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
	
		case DIFF_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);
			(lval.type == REGT) ? sprintf(sreg1, "$t%d", lval.value) : sprintf(sreg1, "%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);
			
			/*OP*/
			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);

			*code = addQuad(*code, "DIFF", sreg, sreg1, sreg2);
			
			ret = (exp){.type = REGT, .value = nreg};
//			return ret;
			break;
			
		case ASSIGN_K:
			lval = genOp(code, tree->child[0]);
			rval = genOp(code, tree->child[1]);

			sprintf(sreg1, "$t%d", lval.value);
			(rval.type == REGT) ? sprintf(sreg2, "$t%d", rval.value) : sprintf(sreg2, "%d", rval.value);

			*code = addQuad(*code, "ASSIGN", sreg1, sreg2, NULL);
			*code = addQuad(*code, "STORE", getVar(lval.value), sreg1, NULL);

			ret = (exp){.type = LITT, .value = 1};
	//		return ret;
			break;


		case NUM_K:
			ret = (exp){.type = LITT, .value = atoi(tree->label)};
	//		return ret;
			break;
		
		case VAR_K:
			int nnreg = getReg(tree->label);

			if(nnreg < 0){
				nreg = linkReg(tree->label);
				sprintf(sreg, "$t%d", nreg);
				*code = addQuad(*code, "LOAD", sreg, tree->label, NULL);
			}
			else
				nreg = nnreg;

			ret = (exp){.type = REGT, .value = nreg};

			return ret;
			break;
		
		case CALL_K:
			astNo* aux;
			char sreg[10], sreg1[10];
			int sz = 0;
			exp val;

			aux = (tree->child) ? tree->child[0] : NULL;
			if(!symTLook(headEnv, "ID", "Function", tree->label, NULL))
				printf("Erro semantico : Funcao %s nao declarada : Linha : %d\n", tree->label, yylineno);

			while(aux){
				val = genOp(code, aux);
				(val.type == REGT) ? sprintf(sreg, "$t%d", val.value) : sprintf(sreg, "%d", val.value);
				*code = addQuad(*code, "PARAM", sreg, NULL, NULL);
				sz++;
				aux = aux->sibling;
			}

			nreg = linkReg(NULL);
			sprintf(sreg, "$t%d", nreg);
			sprintf(sreg1, "%d", sz);
			*code = addQuad(*code, "CALL", sreg, tree->label, sreg1);

			ret = (exp){.type = REGT, .value = nreg};
	//		return ret;
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
	fprintf(fd, "(%s, %s, %s, %s)\n", code->op, code->arg1, code->arg2, code->result);

	// Calls next
	save(code->next, fd);
	return;
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

