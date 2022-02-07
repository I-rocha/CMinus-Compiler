#ifndef AST_H
#define AST_H

#define AST_ERR_0 "ERROR-0: Fail to allocate no\n"
#define AST_ERR_1 "ERROR-1: Impossible to allocate child space\n"
#define AST_ERR_2 "ERROR-2: Grammar ID unknow\n"
#define TAB 3

typedef struct ast* ast;
typedef enum  grammID grammID ;
typedef enum  terminalID terminalID ;

enum grammID{
	kact, karg_lista, kterm, kmult, ksum, kfact, ksoma_exp, krel, ksimple_exp, kvar,	// 0-9
	kexp, kretorno_decl, kiteracao_decl, kselecao_decl, kexp_decl, kstatement_lista, klocal_decl, kcomposto_decl, kparam,	
	kparam_lista, kparams, kfun_decl, ktipo_esp, kvar_decl, kdecl_lista, NIL, terminal	
};

enum terminalID{
	kif, kelse, kint, kreturn, kvoid, kwhile, kop, kID, kNUM, NONE
};

struct ast{
	terminalID termTok;
	grammID tok;
	ast* children;
	int n_child;
	
	// Terminais
	int val;
	char* name;

};

// TODO: Change tokens

ast createNo(grammID mytk);
ast createNoTerminal(terminalID mytk);
int childrenSpace(ast no, int n_child);
void printTree(ast no, int depth);
void printSpace(int nSpace);
void freeTree(ast no);
void print(ast no);

#endif
