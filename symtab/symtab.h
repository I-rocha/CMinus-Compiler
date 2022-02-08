#define H_ERR_0 "H_ERR-0: Comparing nil\n"
#define H_ERR_1 "H_ERR-1: Lookin for nil\n"
#define H_ERR_2 "H_ERR-2: NIL name\n"
#define H_ERR_3 "H_ERR-3: NIL symbol\n"
#define H_ERR_4 "H_ERR-4: Not var and not func symbol\n"
#define H_ERR_5 "H_ERR-5: Scope nil"
#define H_ERR_6 "H_ERR-6: Type nil"
#define GLOBAL "global"
#define ERR_SEM "ERRO SEMANTICO: "
#include "../semantic_global.h"

#define H_MAX 27
typedef struct symbol* symbol;
typedef struct attr attr;
typedef struct ast* ast;


/*	TAB */
symbol hash[H_MAX];
int addNo(symbol sym);
symbol getNo(symbol no);

// OP
int isNameEqual(symbol symA, symbol symB);
int isScopeEqual(symbol symA, symbol symB);
int isEqual(symbol symA, symbol symB);
int exist(char* name, char* scope);
char* getType(char* name, char* scope);
// PRINT
int printAll();
int printContent(symbol el);

