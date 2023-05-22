%{
	#include<stdio.h>
	#include<stdlib.h>
	#include "ast/ast.h"
	#include "parser.tab.h"
	
	extern FILE* fl;
	int lexError();
%}

CSYMBOL \+|\-|\*|\/|\<|\>|\=|\;|\,|\(|\)|\[|\]|\{|\}  

CHAR [a-zA-Z]
DIGIT [0-9]

ID {CHAR}+
NUM {DIGIT}+

COMMENT \/\*([^\/]|\n|\t)*\*\/

%option yylineno noyywrap

%%

{COMMENT} {fprintf(fl, "COMMENT: %s\n", yytext);}


if {
	fprintf(fl, "KEY: if\n");
	return IF;
	}

else {
	fprintf(fl, "KEY: else\n");
	return ELSE;
	}

int {
	fprintf(fl, "KEY: int\n");
	return INT;
	}

return {
	fprintf(fl, "KEY: return\n");
	return RETURN;
	}

void {
	fprintf(fl, "KEY: void\n");
	return VOID;
	}

while {
	fprintf(fl, "KEY: While\n");
	return WHILE;
	}

\>\= {
	fprintf(fl, "SYMBOL: GE\n");
	return GE;
	}

\<\= {
	fprintf(fl, "SYMBOL: LE\n");
	return LE;
	}
\=\= {
	fprintf(fl, "SYMBOL: EQ\n");
	return EQ;
	}

\!\= {
	fprintf(fl, "SYMBOL: DIFF\n");
	return DIFF;
	}

{CSYMBOL} {
	fprintf(fl, "SYMBOL: %c\n", yytext[0]);
	return yytext[0];
	}

{ID} {
	fprintf(fl, "ID: %s\n", yytext);
	yylval.s = strdup(yytext);
	return ID;
	}

{NUM} {
	fprintf(fl, "NUM: %s\n", yytext);
	yylval.val = atoi(yytext);
	return NUM;
	}

[\t ]+ {}

\n {}

. {
	fprintf(fl, "Error, unknown token\n");/*TODO: Verificar casos de espaços*/
	lexError();
}

%%

int lexError(){
	printf("Erro léxico : %s : linha %d\n", yytext, yylineno);
	exit(0);
	return 1;
}

/*
int main(int argc, char **argv){
	FILE* fx;
	if (!(fx = fopen("input/example3.txt", "r"))){
		printf("EOF\n");
		return 0;
	}
	
	yyin = fx;
	yylex();
	printf("lines: %d\n", yylineno);
	printf("EOF\n");
	
	return 0;
}
*/
//int yywrap(){return 0;}
