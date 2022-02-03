#define H_ERR_0 "0-H_ERR: Comparing nil\n"
#define H_ERR_1 "1-H_ERR: Lookin for nil\n"
#define H_MAX 27


struct symbol;
typedef struct symbol* symbol;


//TODO: Add vector 
//TODO: Change escopo name to scope
struct symbol{
	int ival;
	char* escopo;
	char* name;
	char* type;
	symbol prox;
};

symbol head[H_MAX];

int isEqual(symbol a, symbol b);
int hashFunction(char* name);

int add(char* scope, char* type, char* name, int val);
int addNo(symbol no);
symbol get(char* scope, char* name);
symbol getNo(symbol no);

int printAll();
int printContent(symbol el);
