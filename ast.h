#ifndef AST_H
#define AST_H

#define AST_ERR_0 "ERROR-0: Fail to allocate no\n"
#define AST_ERR_1 "ERROR-1: Impossible to allocate child space\n"
#define AST_ERR_2 "ERROR-2: Grammar ID unknow\n"
#define TAB 3
#include "semantic_global.h"

typedef struct symbol* symbol;
typedef struct attr attr;
typedef struct ast* ast;
typedef enum  grammID grammID ;
typedef enum  terminalID terminalID ;


ast createNo(grammID mytk, int line);
ast createNoTerminal(terminalID mytk, int line);
int childrenSpace(ast no, int n_child);
void printTree(ast no, int depth);
void printSpace(int nSpace);
void freeTree(ast no);
void print(ast no);

#endif
