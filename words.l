%{
#include"y.tab.h"
#include "symtab/symtab.h"

struct var;
typedef struct var* var;

struct var{
	char* name;
	int val;
	var prox;
};

//var head=NULL;


%}

C_SYMBOL \+|\-|\*|\/|\<|\>|\=|\;|\,|\(|\)|\[|\]|\{|\}|\/\*|\*\/  

CHAR [a-zA-Z]
DIGIT [0-9]

ID {CHAR}+
NUM {DIGIT}+

COMMENT \/\*.*\*\/

%%

{COMMENT} {printf("Comment:\n%s\n", yytext);}


if {printf("KEY: if\n");
	return IF;}

else {printf("KEY: else\n");
	return ELSE;}

int {printf("KEY: int\n");
	return INT;}

return {printf("KEY: return\n");
	return RETURN;}

void {printf("KEY: void\n");
	return VOID;}

while {printf("KEY: while\n");
	return WHILE;}

\<\= {printf("SYMBOL: LE\n");
	return LE;}

\>\= {printf("SYMBOL: GE\n");
	return GE;}

\=\= {printf("SYMBOL: EQ\n");
	return EQ;}

\!\= {printf("SYMBOL: DIFF\n");
	return DIFF;}

{C_SYMBOL} {printf("Symbol: %s\n", yytext);
	return yytext[0];}

{ID} {printf("ID: %s\n", yytext);
	yylval.s = strdup(yytext);
	return ID;}

{NUM} {printf("NUM: %s\n", yytext);
	yylval.val = atoi(yytext);
	return NUM;}

[\t ]+ |
\n |
. {/*TODO: Verificar casos de espaços*/}

%%

/*TODO: Adicionar erros léxicos*/

