%{
#include <stdio.h>
#include <string.h>
#include "symtab/symtab.h"
#include "defines.h"
#include "ast.h"

void yyerror(char* err);
extern int yylex(void);

ast ast_root = NULL;
%}
%union{
	int val;
	char* s;
	char* token;
	struct symbol* symb;
	struct ast* ast_type;
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
%type <ast_type>termo fator mult
%%

programa:	
		decl_lista{printf("Program matched\n");}

decl_lista:
		  decl_lista decl
		  | decl

decl:
	var_decl
	| fun_decl

var_decl:
		tipo_esp ID ';'	
		| tipo_esp ID '[' NUM ']' ';'

tipo_esp:
		INT	
		| VOID	

fun_decl:
		tipo_esp ID '(' params ')' composto_decl

params:
	  param_lista 
	  | VOID

param_lista:
		   param_lista ',' param
		   | param
		   
param:
	 tipo_esp ID
	 | tipo_esp ID '['']'


composto_decl:
			'{' local_decl statement_lista '}'

local_decl:
		  local_decl var_decl
		  | /*epsilon*/;

statement_lista:
			   statement_lista statement 
			   | /*epsilon*/;

statement:
		 exp_decl
		 | composto_decl
		 | selecao_decl
		 | iteracao_decl
		 | retorno_decl

exp_decl:
		exp ';'
		| ';'

selecao_decl:
	IF '(' exp ')' in_if

in_if:
	statement
	| ELSE statement


iteracao_decl:
			 WHILE '(' exp ')' statement

retorno_decl:
			RETURN ';'
			| RETURN exp ';'

exp:
   var '=' exp
   | simple_exp

var:
   ID
   | ID '[' exp ']'

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

soma:
	'+' 	
	| '-'	


termo:
	 termo mult fator{
	 printf("Termo mult fator\n");
		$2->children = (ast*)malloc(sizeof(ast) * 2);
		$2->children[0] = $1;
		$2->children[1] = $3;
		$2->n_child = 2;
	 	
		$$ = $2;
		ast_root = $2;
	} 
	| fator {
		$$ = $1;
	}


mult:
	'*'	{$$ = storeNo("*", NULL, 0);}
	| '/'{$$ = storeNo("/", NULL, 0);}

fator:
	 '(' exp ')'	{}
	 | var	{}
	 | act	{}
	 | NUM	{$$ = storeNo("NUM", NULL, 0);}

act:
   ID '(' args ')'	{
   						

					}

args:
	arg_lista
	|;

arg_lista:
		 arg_lista ',' exp
		 | exp

%%

void yyerror(char* err){
	printf(ERR_SYN, yylval.token, lineno);
}

int yywrap(){
	printf("Abstract syntax tree:\n");
	preorderWalk(ast_root, 0);
	return 1;
}

int main(){
	yyparse();
	return 1;
}

