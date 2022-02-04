#ifndef AST_H
#define AST_H

#define AST_ERR_0 "AST_ERR_TRAVERSAL: child null\n"
#define AST_ERR_1 "AST_ERR_CONCAT: null\n"
#define WHITE_SP 3

typedef struct ast* ast;

struct ast{
	char* name;
	ast* children;
	int n_child;		// Child quantity
};

ast storeNo(char* noName, ast* children, int n_child);
int preorderWalk(ast no, int depth);
void printNested(char* msg, int ntab);
int freeDesc(ast no);
void freeA(ast no);
int children_concat(ast chA, ast chB);
int addChild(ast no, ast val);

#endif
