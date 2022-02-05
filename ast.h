#ifndef AST_H
#define AST_H

#define AST_ERR_0 "ERROR-0: Fail to allocate no\n"
#define AST_ERR_1 "ERROR-1: Impossible to allocate child space\n"
#define TAB 3

typedef struct ast* ast;
typedef enum token ttoken;

enum token{kterm, kmult, ksum, kfact, ksoma_exp, krel, ksimple_exp};

struct ast{

	ttoken tok;
	ast* children;
	int n_child;
};

// TODO: Change tokens

ast createNo(ttoken mytk);
int childrenSpace(ast no, int n_child);
void printTree(ast no, int depth);
void printSpace(int nSpace);
void freeTree(ast no);

#endif
