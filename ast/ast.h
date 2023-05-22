#ifndef AST_H
#define AST_H

typedef enum token Token;

enum token{
	BLANK = 0,
	ALLOC_K,
	ALLOC_ARRAY_K,
	ARRAY_SIZE_K,
	INT_K,
	VOID_K,
	FUN_K,
	ARG_K,
	ARG_ARRAY_K,
	IF_K,
	WHILE_K,
	RETURN_K,
	ASSIGN_K,
	VAR_K,
	VAR_ARRAY_K,
	NUM_K,
	CALL_K,

	// REL
	LEQ_K,
	LESS_K,
	GEQ_K,
	GRAND_K,
	EQ_K,
	DIFF_K,
	
	// OP
	PLUS_K,
	MINUS_K,
	MULT_K,
	DIV_K
};


typedef struct astNo{
	Token label;
	struct astNo* sibling;
	struct astNo** child;
	
	// Children len
	int len_child;
}astNo;

/*
 * Begin struct with empty*/
astNo* astInit();

/*
 * Creates allocated children of size sz
 * 1: successfull
 * -1: error
 * */

//int astCreateChild(astNo* no, int sz);

/*
 * References multiple children to child
 * WARN - If for any i'th child target[i] is NULL
 * then target[i] is not assigned and sz is automatically decreased
 * sz: Quantity of children
 * 1: successfull
 * 0: nothing happened
 * -1: error
 * */
int astPutChild(astNo* no, astNo** target, int sz);

/*
 * Allocates no to sibling
 * 1: successfull
 * 0: Nothing happened
 * -1: error
 * */
int astInsertSibling(astNo* no, astNo target);

/*
 * Reference multiple *Target to siblings
 * If some target[i] is null, target[i] is not assigned
 * len: quantity of sibling to put
 * 1: successfull
 * -1: error
 * */
int astPutSibling(astNo* no, astNo** target, int len);

/*
 * Assign data to no without allocate it. The data inside is allocated (except child)
 * (astNo): copy of no created
 * */
astNo astNewNo(Token label,astNo** child, int len_child);

/*
 * Assign data allocating it. The data inside is also allocated (except child)
 * (astNo*): addr of no created and allocated
 * */
astNo* astCreateNo(Token label, astNo** child, int len_child);

/*
 * Free entire tree*/
int astFree(astNo* tree);

/*
 * Free deep inside tree*/
int astDeepFree(astNo* no);

/*
 * Print the tree with important info*/
void astPrintInfo(astNo* no);

/*
 * Print the tree*/
void astPrint(astNo* no);

/* 
 * Save at file given by path*/
int astSave(astNo* tree, char* path);

// TEMP
char* tokenStr(Token tok);

#endif
