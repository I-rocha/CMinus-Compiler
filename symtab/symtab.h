#define H_ERR_0 "H_ERR-0: Comparing nil\n"
#define H_ERR_1 "H_ERR-1: Lookin for nil\n"
#define H_ERR_2 "H_ERR-2: NIL name\n"
#define H_ERR_3 "H_ERR-3: NIL symbol\n"
#define H_ERR_4 "H_ERR-4: Not var and not func symbol\n"
#define H_ERR_5 "H_ERR-5: Scope nil"
#define GLOBAL "global"

#define H_MAX 27


struct symbol;
typedef struct symbol* symbol;
typedef struct attr attr;

struct attr{
	char* type;
	char* name;
	char* scope;
	int var_func;
};

//TODO: Add vector 
struct symbol{
	attr content;
	symbol prox;
};


int hashFunction(char* name);

/*	TAB */
symbol hash[H_MAX];
int addNo(symbol sym);
symbol getNo(symbol no);

// OP
int isNameEqual(symbol symA, symbol symB);
int isScopeEqual(symbol symA, symbol symB);
int isEqual(symbol symA, symbol symB);
int exist(char* name, char* scope);

// ERROR CHECK
int checkDeclarationFunc(symbol sfunc);
int checkDeclarationVar(symbol svar);
int checkFunc(symbol sfunc);
int checkVar(symbol svar);

// PRINT
int printAll();
int printContent(symbol el);

