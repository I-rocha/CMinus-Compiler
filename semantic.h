#ifndef SEMANTIC_H
#define SEMANTIC_H

#define GLOBAL "global"
#define FUNC 0
#define VAR 1

#define ERR_SEM_0 "AST-SEM ERR0: NULL lookin for token\n"

typedef struct attr attr;

struct attr{
	char* type;
	char* name;
	char* scope;
	int var_func;
};

void table(ast root);
void getTokens(ast root, attr* currAttr, char* currScope);
ast nextTerminal(ast no);

void m_fun_decl(ast root, attr* currAttr, char* currScope);
void m_var_decl(ast root, attr* currAttr, char* currScope);
void m_varf_decl(ast root, attr* currAttr, char* currScope);
void m_fun_act(ast root, attr* currAttr, char* currScope);
void m_var(ast root, attr* currAttr, char* currScope);

#endif
