%{
#include <stdio.h>
#include <string.h>
#include "symtab/symtab.h"

void yyerror(char* err);
extern int yylex(void);
%}

%union{
	int val;
	char* s;
	struct symbol* symb;
}
/**/
// KEYWORDS
%token IF ELSE INT RETURN VOID WHILE LE GE EQ DIFF

// OP -> Default declaration

// ID
%token <s>ID

// NUM
%token <val>NUM

// Terminal types
//%type <val>simple_exp <val>soma_exp <val>soma <val>termo <val>mult <val>fator <val>rel
//%type <symb>tip_esp <symb> var_decl
%%

//TODO: Remover esse regex
tmp:
  simple_exp 
  | var_decl 

var_decl:
		tip_esp ID ';'	
		| tip_esp ID '[' NUM ']' ';'

tip_esp:
		INT	
		| VOID	

simple_exp:
		  soma_exp rel soma_exp	
		  | soma_exp

rel:
   LE
   |'<'	
   |'>'	
   |GE	
   |EQ	
   |DIFF

soma_exp:
		soma_exp soma termo 
		| termo 
		;

soma:
	'+' 	
	| '-'	
	;


termo:
	 termo mult fator 
	| fator 
	;

mult:
	'*'	
	| '/'
	;

/*TODO: Complementar regex*/
fator:
	 NUM 
	 ;

%%

void yyerror(char* err){
	printf("Parser error: %s", err);
}

int yywrap(){
	return 1;
}

int main(){
	yyparse();
	return 1;
}

