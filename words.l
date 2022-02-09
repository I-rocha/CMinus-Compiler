%{
#include"y.tab.h"
#include "symtab/symtab.h"
#include "defines.h"
#include "file_log.h"

//TODO: Change ID to accept digit after first char

// Defining line count

%}

CSYMBOL \+|\-|\*|\/|\<|\>|\=|\;|\,|\(|\)|\[|\]|\{|\}  

CHAR [a-zA-Z]
DIGIT [0-9]

ID {CHAR}+
NUM {DIGIT}+

COMMENT \/\*(.|\n|\t)*\*\/

%option yylineno
%option noyywrap
%%

{COMMENT} {printf("Comment:\n%s\n", yytext);}


if {
	fprintf(scan_log, "%s", "KEY: if\n");
	yylval.token = strdup("KEY");
	return IF;
	}

else {
	fprintf(scan_log, "%s", "KEY: else\n");
	yylval.token = strdup("KEY");
	return ELSE;
	}

int {
	fprintf(scan_log, "%s", "KEY: int\n");
	yylval.token = strdup("KEY");
	return INT;
	}

return {
	fprintf(scan_log, "%s", "KEY: return\n");
	yylval.token = strdup("KEY");
	return RETURN;
	}

void {
	fprintf(scan_log, "%s", "KEY: void\n");
	yylval.token = strdup("KEY");
	return VOID;
	}

while {
	fprintf(scan_log, "%s", "KEY: while\n");
	yylval.token = strdup("KEY");
	return WHILE;
	}

\<\= {
	fprintf(scan_log, "%s", "SYMBOL: LE\n");
	yylval.token = strdup("<=");
	return LE;
	}

\>\= {
	fprintf(scan_log, "%s", "SYMBOL: GE\n");
	yylval.token = strdup(">=");
	return GE;
	}

\=\= {
	fprintf(scan_log, "%s", "SYMBOL: EQ\n");
	yylval.token = strdup("==");
	return EQ;
	}

\!\= {
	fprintf(scan_log, "%s", "SYMBOL: DIFF\n");
	yylval.token = strdup("!=");
	return DIFF;
	}

{CSYMBOL} {
	fprintf(scan_log, "%s %s\n", "SYMBOL:",yytext);
	//printf("SYMBOL: %s\n", yytext);
	yylval.token = strdup(&yytext[0]);
	return yytext[0];
	}

{ID} {
	//printf("ID: %s\n", yytext);
	fprintf(scan_log, "%s: %s\n", "ID",yytext);
	yylval.token = strdup("ID");
	yylval.s = strdup(yytext);
	return ID;
	}

{NUM} {
	//printf("NUM: %s\n", yytext);
	fprintf(scan_log, "%s: %d\n", "NUM", atoi(yytext));
	yylval.token = strdup("NUM");
	yylval.val = atoi(yytext);
	return NUM;
	}

[\t ]+ {}

\n {}

. {printf(ERR_LEX, yytext, yylineno);/*TODO: Verificar casos de espaços*/}

%%

/*TODO: Adicionar erros léxicos*/
