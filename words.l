%{
#include"y.tab.h"
#include "symtab/symtab.h"
#include "defines.h"

/*struct var;
typedef struct var* var;

struct var{
	char* name;
	int val;
	var prox;
};
*/
//TODO: Change ID to accept digit after first char

// Defining line count
int lineno = 1;

%}

CSYMBOL \+|\-|\*|\/|\<|\>|\=|\;|\,|\(|\)|\[|\]|\{|\}|\/\*|\*\/  

CHAR [a-zA-Z]
DIGIT [0-9]

ID {CHAR}+
NUM {DIGIT}+

COMMENT \/\*.*\*\/

%%

{COMMENT} {printf("Comment:\n%s\n", yytext);}


if {
	printf("KEY: if\n");
	yylval.token = strdup("KEY");
	return IF;
	}

else {
	printf("KEY: else\n");
	yylval.token = strdup("KEY");
	return ELSE;
	}

int {
	printf("KEY: int\n");
	yylval.token = strdup("KEY");
	return INT;
	}

return {
	printf("KEY: return\n");
	yylval.token = strdup("KEY");
	return RETURN;
	}

void {
	printf("KEY: void\n");
	yylval.token = strdup("KEY");
	return VOID;
	}

while {
	printf("KEY: while\n");
	yylval.token = strdup("KEY");
	return WHILE;
	}

\<\= {
	printf("SYMBOL: LE\n");
	yylval.token = strdup("<=");
	return LE;
	}

\>\= {
	printf("SYMBOL: GE\n");
	yylval.token = strdup(">=");
	return GE;
	}

\=\= {
	printf("SYMBOL: EQ\n");
	yylval.token = strdup("==");
	return EQ;
	}

\!\= {
	printf("SYMBOL: DIFF\n");
	yylval.token = strdup("!=");
	return DIFF;
	}

{CSYMBOL} {
	printf("SYMBOL: %s\n", yytext);
	yylval.token = strdup(&yytext[0]);
	return yytext[0];
	}

{ID} {printf("ID: %s\n", yytext);
	yylval.token = strdup("ID");
	yylval.s = strdup(yytext);
	return ID;
	}

{NUM} {
	printf("NUM: %s\n", yytext);
	yylval.token = strdup("NUM");
	yylval.val = atoi(yytext);
	return NUM;
	}

[\t ]+ {}

\n {lineno++;}

. {printf(ERR_LEX, yytext, lineno);/*TODO: Verificar casos de espaços*/}

%%

/*TODO: Adicionar erros léxicos*/
