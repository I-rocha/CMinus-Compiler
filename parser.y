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
%token <int>IF <int>ELSE INT RETURN VOID WHILE LE GE EQ DIFF <s>ID <val>NUM 

%type <ast_no> programa decl_lista decl var_decl fun_decl tipo_esp params composto_decl param_lista param local_decl statement_lista statement exp_decl selecao_decl iteracao_decl retorno_decl exp var simple_exp soma_exp rel soma termo mult fator act args arg_lista else_stmt

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
	astNo* aux[] = {astCreateNo(ALLOC_K, NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Symbol Table
	symTPut(env, VAR_K, $2, $1->label, 0, yylineno);

	// Free
	free($2);
	}
	| tipo_esp ID '[' NUM ']' ';'	{
	// Adding array to symbol table
	astNo* aux[] = {astCreateNo(ALLOC_ARRAY_K, NULL, 0)};
	astNo* aux2[] = {astCreateNo(ARRAY_SIZE_K, NULL, 0)};
	astPutChild($1, aux, 1);
	astPutChild($1->child[0], aux2, 1);
	
	// Symbol Table
	symTPut(env, VAR_ARRAY_K, $2, $1->label, $4, yylineno);

	$$ = $1;
	free($2);
	}
	;

tipo_esp:
	INT{
	$$ = astCreateNo(INT_K, NULL, 0);
	}
	|
	VOID{
	$$ = astCreateNo(VOID_K, NULL, 0);
	}
	
	;
		
fun_decl:
	tipo_esp ID 
	{
		symTPut(headEnv, FUN_K, $2, $1->label, 0, yylineno);	// Add to env
		env = symTNewEnv(env, $2);		// New env
	}
	
	'(' params ')' composto_decl	{

	// ID is child of tipo_esp
	astNo* aux[] = {astCreateNo(FUN_K, NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Params and composto_decl are children of ID
	astNo* aux2[] = {$5, $7};
	astPutChild($1->child[0], aux2, 2);

	env = symTExit(env);

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
	astNo* aux[] = {astCreateNo(ARG_K, NULL, 0)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Symbol Table
	symTPut(env, VAR_K, $2, $1->label, 0, yylineno);

	// Free
	free($2);
	}
	| tipo_esp ID '['']'	{
	// Add to symbol table
	symTPut(env, VAR_K, $2, $1->label, 0, yylineno);

	astNo* aux[] = {astCreateNo(ARG_ARRAY_K, NULL, 0)};
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
	    IF '(' exp ')' { env = symTNewEnv(env, "IF");} statement {env = symTExit(env);} else_stmt{
		$$ = astCreateNo(IF_K, NULL, 0);
		if($8){
			astNo* aux[] = {$3, $6, $8};
			astPutChild($$, aux, 3);
		}
		else{
			astNo* aux[] = {$3, $6};
			astPutChild($$, aux, 2);
		}
	}
	;

else_stmt:
	%prec IFX {
	$$ = NULL;
	}
	| ELSE {env = symTNewEnv(env, "ELSE");} statement{
	$$ = $3;
	env = symTExit(env);	// Exit env
	}
	;

iteracao_decl:
	WHILE 
	{
		env = symTNewEnv(env, "WHILE");
	}
	'(' exp ')' statement	{
	astNo* aux[] = {$4, $6};
	$$ = astCreateNo(WHILE_K, NULL, 0);
	astPutChild($$, aux, 2);

	env = symTExit(env); // Exit env
	}
	;

retorno_decl:
	RETURN ';'	{$$ = astCreateNo(RETURN_K, NULL, 0);}
	| RETURN exp ';'	{
	astNo* aux[] = {$2};
	$$ = astCreateNo(RETURN_K, NULL, 0);
	astPutChild($$, aux, 1);
	}
	;
exp:	
	var '=' exp	{
	astNo* aux[] = {$1, $3};
	$$ = astCreateNo(ASSIGN_K, NULL, 0);
	astPutChild($$, aux, 2);
	}
	| simple_exp	{$$ = $1;}
	;
	
var:
	ID	{
	// AST
	$$ = astCreateNo(VAR_K, NULL,0);

	// Symtab
	symTAddRef(env, $1, yylineno);

	// Free
	free($1);
	
	}	
	| ID '[' exp ']'	{
	astNo* aux[] = {$3};
	$$ = astCreateNo(VAR_ARRAY_K, NULL, 0);
	astPutChild($$, aux, 1);
	
	// Symtab
	symTAddRef(env, $1, yylineno);
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
 	LE	{$$ = astCreateNo(LEQ_K, NULL, 0);}
	|'<'	{$$ = astCreateNo(LESS_K, NULL, 0);}
	|'>'	{$$ = astCreateNo(GRAND_K, NULL, 0);}
	|GE	{$$ = astCreateNo(GEQ_K, NULL, 0);}
	|EQ	{$$ = astCreateNo(EQ_K, NULL, 0);}
	|DIFF	{$$ = astCreateNo(DIFF_K, NULL, 0);}
	;

soma_exp:
		soma_exp soma termo {
		astNo *aux[2] = {$1, $3};
		astPutChild($2, aux, 2);
		$$ = $2;
		}
	| termo	{$$ = $1;} ;

soma:
	'+'	{$$ = astCreateNo(PLUS_K, NULL, 0);}
	| '-'	{$$ = astCreateNo(MINUS_K, NULL, 0);};

termo:
	termo mult fator {
	astNo* aux[] = {$1, $3};
	astPutChild($2, aux, 2);
	$$ = $2;
	}
	| fator {$$ = $1;} ;


mult:
	'*'	{$$ = astCreateNo(MULT_K, NULL, 0);}
	| '/'	{$$ = astCreateNo(DIV_K, NULL, 0);}
	;
fator:
	 '(' exp ')'  {$$ = $2;}
	| var  {$$ = $1;}
	| act	{$$ = $1;}
	| NUM	{
	$$ = astCreateNo(NUM_K, NULL, 0);
	}
	;
act:
   ID '(' args ')' {
	// AST
	astNo* aux[] = {$3};
	$$ = astCreateNo(CALL_K, NULL, 0);
	astPutChild($$, aux, 1);

	// Symtab
	symTAddRef(env, $1, yylineno);

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

