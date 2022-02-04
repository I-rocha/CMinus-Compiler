#ifndef AST_H
#define AST_H

#define AST_ERR_0 "AST_ERR_TRAVERSAL: child null\n"
#define WHITE_SP 3

typedef struct ast* ast;

struct ast{
	char* name;
	ast* children;
	int n_child;		// Child quantity
};

extern ast ast_root;

ast storeNo(char* noName, ast* children, int n_child);
int preorderWalk(ast no, int depth);
void printNested(char* msg, int ntab);


#endif
