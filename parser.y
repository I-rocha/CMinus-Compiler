%{
#include <stdio.h>
#include <string.h>
#include "symtab/symtab.h"
#include "defines.h"
#include "ast.h"

void yyerror(char* err);
extern int yylex(void);

ast ast_root;
%}

%union{
	int val;
	char* s;
	char* token;
	struct symbol* symb;
	struct ast* t_ast;
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

/*TODO: Organizar types*/
%type <t_ast> /*programa decl_lista decl var_decl tipo_esp fun_decl params param_lista param composto_decl local_decl statement_lista statement exp_decl selecao_decl in_if iteracao_decl retorno_decl exp var*/ simple_exp rel soma_exp soma termo mult fator /*act args arg_lista*/

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
		  soma_exp rel soma_exp	{$$ = createNo(ksimple_exp);}
		  | soma_exp	{$$ = $1;}

rel:
   LE	{$$ = createNo(krel);}
   |'<'	{$$ = createNo(krel);}
   |'>'	{$$ = createNo(krel);}
   |GE	{$$ = createNo(krel);}
   |EQ	{$$ = createNo(krel);}
   |DIFF	{$$ = createNo(krel);}


soma_exp:
		soma_exp soma termo  {
		$$ = createNo(ksoma_exp);
		childrenSpace($$, 3);
		$$->children[0] = $1;
		$$->children[1] = $2;
		$$->children[2] = $3;
		} 
		| termo	{$$ = $1;} 

soma:
	'+'		{$$ = createNo(ksum);}
	| '-'	{$$ = createNo(ksum);}


termo:
	 termo mult fator {
     $$ = createNo(kterm);
	 childrenSpace($$, 3);
	 $$->children[0] = $1;
	 $$->children[1] = $2;
	 $$->children[2] = $3;
	} 
	| fator {$$ = $1;} 


mult:
	'*'		{$$ = createNo(kmult);}
	| '/'	{$$ = createNo(kmult);}

fator:
	 '(' exp ')' {}
	 | var  {}
	 | act	{}
	 | NUM	{$$ = createNo(kfact);}

act:
   ID '(' args ')'

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
	printTree(ast_root, 0);
	freeTree(ast_root);
	return 1;
}

int main(){
	yyparse();
	return 1;
}

