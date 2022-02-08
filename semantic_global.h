#ifndef SEMANTIC_GLOBAL_H
#define SEMANTIC_GLOBAL_H

#include "symtab/symtab.h"
#include "ast.h"


typedef enum  grammID grammID ;
typedef enum  terminalID terminalID ;
typedef struct symbol* symbol;
typedef struct attr attr;
typedef struct ast* ast;


enum grammID{
	kact, karg_lista, kterm, kmult, ksum, kfact, ksoma_exp, krel, ksimple_exp, kvar,	// 0-9
	kexp, kretorno_decl, kiteracao_decl, kselecao_decl, kexp_decl, kstatement_lista, klocal_decl, kcomposto_decl, kparam,	
	kparam_lista, kparams, kfun_decl, ktipo_esp, kvar_decl, kdecl_lista, NIL, terminal	
};

enum terminalID{
	kif, kelse, kint, kreturn, kvoid, kwhile, kop, kID, kNUM, NONE
};

struct attr{
	char* type;
	char* name;
	char* scope;
	int var_func;
	int line;
};

struct ast{
	terminalID termTok;
	grammID tok;
	ast* children;
	int n_child;
	int line;
	
	// Terminais
	int val;
	char* name;

};

struct symbol{
	attr content;
	symbol prox;
};

/* Symbol Table */
int hashFunction(char* name);

#endif
