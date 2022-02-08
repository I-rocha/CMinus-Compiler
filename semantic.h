#ifndef SEMANTIC_H
#define SEMANTIC_H

#define GLOBAL "global"
#define FMAIN "main"
#define FINPUT "input"
#define FOUTPUT "output"
#define TANY "ANY"
#define FUNC 0
#define VAR 1


#define ERR_SEM_0 "AST-SEM ERR0: NULL lookin for token\n"


typedef struct attr attr;

void semantic(ast root);

// run AST
char* lookType(ast root, char* ctype, char* currScope);
void getTokens(ast root, attr* currAttr, char* currScope);
ast nextTerminal(ast no);

// Match functions
void m_fun_decl(ast root, attr* currAttr, char** currScope);
void m_var_decl(ast root, attr* currAttr, char** currScope);
void m_varf_decl(ast root, attr* currAttr, char** currScope);
void m_fun_act(ast root, attr* currAttr, char** currScope);
void m_var(ast root, attr* currAttr, char** currScope);

// ERROR CHECK
int checkDeclarationFunc(ast no, symbol sfunc);
int checkDeclarationVar(ast no, symbol svar);
int checkFunc(ast no, symbol sfunc);
int checkVar(ast no, symbol svar);
int checkType(ast no, char* t1, char* t2);
int checkMain();


#endif
