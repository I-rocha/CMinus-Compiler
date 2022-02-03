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
  programa 
  

programa:
		decl_lista

decl_lista:
		  decl_lista decl
		  | decl

decl:
	var_decl
	| fun_decl

var_decl:
		tipo_esp ID ';'	
		| tipo_esp ID '[' NUM ']' ';'

fun_decl:
		tipo_esp ID '(' params ')'

params:
	  param_lista 
	  | VOID

param_lista:
		   param_lista ',' param
		   | param
		   
param:
	 tipo_esp ID
	 | tipo_esp ID '['']'

tipo_esp:
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

