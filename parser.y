%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"
#include "cgen/cgen.h"
#include "lexical/symtab.h"

int yyerror(char* s);
extern int yylex();
extern int yylineno;
extern astNo* astTree;
extern symTable* env;
extern symTable* headEnv;
%}

%union{
	int val;
	char* s;
	astNo* ast_no;
}

/**/
// KEYWORDS
%token <int>IF <int>ELSE INT RETURN VOID WHILE LE GE EQ DIFF <s>ID <s>NUM 

%type <ast_no> programa decl_lista decl var_decl fun_decl tipo_esp params composto_decl param_lista param local_decl statement_lista statement exp_decl selecao_decl iteracao_decl retorno_decl exp var simple_exp soma_exp rel soma termo mult fator act args arg_lista

%nonassoc IFX
%nonassoc ELSE

%%

programa:			
	decl_lista {
	astTree = $1;
	}
	;

decl_lista:
	decl_lista decl	{
	astNo* aux[] = {$2};
	if($1){
		astPutSibling($1, aux, 1);
		$$ = $1;
	}
	else if($2)
		$$ = $2;
	else
		$$ = NULL;
	}
	
	| decl	{$$ = $1;}
	;

decl:
	var_decl	{$$ = $1;}
	| fun_decl	{$$ = $1;}
	;


var_decl:
	tipo_esp ID ';'{

	// AST
	astNo* aux[] = {astCreateNo($2, "ALLOC", NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Symbol Table
	if(symTPut(env, "ID", "Variable", $2, NULL, $1->label) == 0)
		printf("Erro semantico - Variavel ja declarada : Linha %d\n", yylineno);
	
	if(strcmp($1->label, "VOID") == 0)
		printf("Erro semantico - Variavel tipo VOID : Linha %d\n", yylineno);
	
	// Free
	free($2);
	}
	| tipo_esp ID '[' NUM ']' ';'	{
	// TODO - add array to symbol table
	astNo* aux[] = {astCreateNo($2, "ALLOC_ARRAY", NULL, 0)};
	astNo* aux2[] = {astCreateNo($4, "ARRAY_SIZE", NULL, 0)};
	astPutChild($1, aux, 1);
	astPutChild($1->child[0], aux2, 1);
	$$ = $1;
	}
	;

tipo_esp:
	INT{
	$$ = astCreateNo("INT", "TYPE", NULL, 0);
	}
	|
	VOID{
	$$ = astCreateNo("VOID", "TYPE", NULL, 0);
	}
	
	;
		
fun_decl:
	tipo_esp ID '(' params ')' composto_decl	{
	// ID is child of tipo_esp
	astNo* aux[] = {astCreateNo($2, "FUN", NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Params and composto_decl are children of ID
	astNo* aux2[] = {$4, $6};
	astPutChild($1->child[0], aux2, 2);

	// Symbol Table
	symTPut(headEnv, "ID", "Function", $2, NULL, $1->label);
	//	printf("Erro semantico - Função já declarada : Linha : %d\n", yylineno);		

	env = symTNewEnv(env);

	// Free
	free($2);
	}
	;

params:
	  param_lista	{$$ = $1;}
	| VOID	{$$ = NULL;}	
	;

param_lista:
	param_lista ',' param	{
	astNo* aux[] = {$3};
	astPutSibling($1, aux, 1);
	$$ = $1;
	}
	| param	{$$ = $1;}
	;
		   
param:
	tipo_esp ID	{
	astNo* aux[] = {astCreateNo($2, "ARG", NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Symbol Table
	symTPut(env, "ID", "Variable",  $2, NULL, $1->label);

	// Free
	free($2);
	}
	| tipo_esp ID '['']'	{
	// TODO - add to symbol table
	astNo* aux[] = {astCreateNo($2, "ARG_ARRAY", NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;
	}
	;

composto_decl:
	'{' local_decl statement_lista '}'	{
	astNo* aux[] = {$3};
	if($2){
		astPutSibling($2, aux, 1);
		$$ = $2;
	}

	else if($3)
		$$ = $3;

	else
		$$ = NULL;
	}
	;

local_decl:
	local_decl var_decl	{
	astNo* aux[] = {$2};
	if($1){
		astPutSibling($1, aux, 1);
		$$ = $1;
	}
	else{
		$$ = $2;
	}
	}
	| /*epsilon*/{$$ = NULL;};
	

statement_lista:
	statement_lista statement	{
	astNo* aux[] = {$2};

	if($1){
		astPutSibling($1, aux, 1);
		$$ = $1;
	}
	else
		$$ = $2;
	}
	| /*epsilon*/{$$ = NULL;}
	;

statement:
		 exp_decl	{$$ = $1;}
		 | composto_decl	{$$ = $1;}
		 | selecao_decl	{$$ = $1;}
		 | iteracao_decl	{$$ = $1;}
		 | retorno_decl	{$$ = $1;}
		;

exp_decl:
	exp ';'	{$$ = $1;}
	| ';'	{$$ = NULL;}
	;

selecao_decl:
	IF '(' exp ')' statement %prec IFX	{
	astNo* aux[] = {$3, $5};
	$$ = astCreateNo("IF", "IF", NULL, 0);
	astPutChild($$, aux, 2);
	} 
	| IF '(' exp ')' statement ELSE statement{
	astNo* aux[] = {$3, $5, $7};
	$$ = astCreateNo("IF", "IF", NULL, 0);
	astPutChild($$, aux, 3);
	}
	;

iteracao_decl:
	WHILE '(' exp ')' statement	{
	astNo* aux[] = {$3, $5};
	$$ = astCreateNo("WHILE","WHILE", NULL, 0);
	astPutChild($$, aux, 2);
	}
	;

retorno_decl:
	RETURN ';'	{$$ = astCreateNo("RETURN","RETURN", NULL, 0);}
	| RETURN exp ';'	{
	astNo* aux[] = {$2};
	$$ = astCreateNo("RETURN","RETURN",NULL, 0);
	astPutChild($$, aux, 1);
	}
	;
exp:	
	var '=' exp	{
	astNo* aux[] = {$1, $3};
	$$ = astCreateNo("=", "ASSIGN", NULL, 0);
	astPutChild($$, aux, 2);
	}
	| simple_exp	{$$ = $1;}
	;
	
var:
	ID	{
	// AST
	$$ = astCreateNo($1,"VAR",NULL,0);

	// Symbol Table
	if(!symTLook(env, "ID", "Variable", $1, NULL))	
		printf("Erro Semantico - Variável %s não definida : Linha %d\n", $1, yylineno);

	// Free
	free($1);
	
	}	
	| ID '[' exp ']'	{
	//TODO: Add array to symbol table
	astNo* aux[] = {$3};
	$$ = astCreateNo($1, "VAR_ARRAY", NULL, 0);
	astPutChild($$, aux, 1);
	}
	;

simple_exp:
	soma_exp rel soma_exp	{
	astNo* aux[] = {$1, $3};
	astPutChild($2, aux, 2);
	$$ = $2;
	}
	| soma_exp	{$$ = $1;}
	;

rel:
 	LE	{$$ = astCreateNo("LE","REL", NULL, 0);}
	|'<'	{$$ = astCreateNo("<","REL", NULL, 0);}
	|'>'	{$$ = astCreateNo(">","REL", NULL, 0);}
	|GE	{$$ = astCreateNo("GE","REL", NULL, 0);}
	|EQ	{$$ = astCreateNo("EQ","REL", NULL, 0);}
	|DIFF	{$$ = astCreateNo("DIFF","REL", NULL, 0);}
	;

soma_exp:
		soma_exp soma termo {
		astNo *aux[2] = {$1, $3};
		astPutChild($2, aux, 2);
		$$ = $2;
		}
	| termo	{$$ = $1;} ;

soma:
	'+'	{$$ = astCreateNo("+","OP", NULL, 0);}
	| '-'	{$$ = astCreateNo("-", "OP", NULL, 0);};

termo:
	termo mult fator {
	astNo* aux[] = {$1, $3};
	astPutChild($2, aux, 2);
	$$ = $2;
	}
	| fator {$$ = $1;} ;


mult:
	'*'	{$$ = astCreateNo("*", "OP", NULL, 0);}
	| '/'	{$$ = astCreateNo("/","OP", NULL, 0);}
	;
fator:
	 '(' exp ')'  {$$ = $2;}
	| var  {$$ = $1;}
	| act	{$$ = $1;}
	| NUM	{
	$$ = astCreateNo($1, "NUM", NULL, 0);
	free($1);
	}
	;
act:
   ID '(' args ')' {
	// AST
	astNo* aux[] = {$3};
	$$ = astCreateNo($1, "CALL", NULL, 0);
	astPutChild($$, aux, 1);

	// Free
	free($1);
	}
	;

args:
	arg_lista	{$$ = $1;}
	| /*epsilon*/	{$$ = NULL;}	
	;

arg_lista:
	 arg_lista ',' exp	{
	astNo* aux[] = {$3};
	astPutSibling($1, aux, 1);
	$$ = $1;
	}
	| exp	{$$ = $1;}
	;

%%

/*
int main(int argc, char** argv){
	FILE *fp;

	if(argc <=1){
		printf("Please follow pattern: <exe> <file>\n");
		return 0;
	}

	if(!(fp = fopen(argv[1], "r"))){
		printf("Error openning file\n");
		return 0;
	}
	yyin = fp;
	yyparse();

	if(fp)
		fclose(fp);
	
	return 1;
}
*/

int yyerror(char* s){
	printf("Erro sintático : linha %d\n", yylineno);
	exit(0);
	return 1;
}

