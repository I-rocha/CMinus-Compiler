%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"
#include "cgen/cgen.h"
#include "lexical/symtab.h"
#define I2A_SZ 10

int yyerror(char* s);
extern int yylex();
extern int line_log;
extern char* yytext;
extern astNo* astTree;
%}

%union{
	int val;
	char* s;
	astNo* ast_no;
}

/**/
// KEYWORDS
%token <int>IF <int>ELSE INT RETURN VOID WHILE LE GE EQ DIFF <s>ID <val>NUM 

%type <ast_no> programa decl_lista decl var_decl fun_decl tipo_esp params composto_decl param_lista param local_decl statement_lista statement exp_decl selecao_decl iteracao_decl retorno_decl exp var simple_exp soma_exp rel soma termo mult fator act args arg_lista else_stmt <val>EPSLON_LINE

%nonassoc IFX
%nonassoc ELSE

%%

// 1
programa:			
	decl_lista {
	astTree = $1;
	}
	;

// 2
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

// 3
decl:
	var_decl	{$$ = $1;}
	| fun_decl	{$$ = $1;}
	;

// 4
var_decl:
	tipo_esp ID EPSLON_LINE ';'{

	// AST
	astNo* aux[] = {astCreateTerminal(ALLOC_K, $2, NULL, 0, $3)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Free
	free($2);
	}
	| tipo_esp ID EPSLON_LINE '[' NUM ']' ';'{
	char str[I2A_SZ];
	sprintf(str, "%d", $5);

	astNo* aux[] = {astCreateTerminal(ALLOC_ARRAY_K, $2, NULL, 0, $3)};
	astNo* aux2[] = {astCreateNo(ARRAY_SIZE_K, str, NULL, 0)};
	astPutChild($1, aux, 1);
	astPutChild($1->child[0], aux2, 1);
	
	$$ = $1;
	free($2);
	}
	;

EPSLON_LINE:{$$ = line_log;}

// 5
tipo_esp:
	INT{
	$$ = astCreateNo(INT_K, NULL, NULL, 0);
	}
	|
	VOID{
	$$ = astCreateNo(VOID_K, NULL, NULL,  0);
	}
	
	;

// 6	
fun_decl:
	tipo_esp ID {$<val>$ = line_log;}'(' params ')' composto_decl	{
	
	// ID is child of tipo_esp
	astNo* aux[] = {astCreateTerminal(FUN_K, $2, NULL, 0, $<val>3)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Params and composto_decl are children of ID
	astNo* aux2[] = {$5, $7};
	astPutChild($1->child[0], aux2, 2);

	// Free
	free($2);
	}
	;

// 7
params:
	  param_lista	{$$ = $1;}
	| VOID	{$$ = NULL;}	
	;

// 8
param_lista:
	param_lista ',' param	{
	astNo* aux[] = {$3};
	astPutSibling($1, aux, 1);
	$$ = $1;
	}
	| param	{$$ = $1;}
	;

// 9   
param:
	tipo_esp ID EPSLON_LINE	{
	astNo* aux[] = {astCreateTerminal(ARG_K, $2, NULL, 0, $3)};
	astPutChild($1, aux, 1);
	$$ = $1;

	// Free
	free($2);
	}
	| tipo_esp ID EPSLON_LINE '['']'	{

	astNo* aux[] = {astCreateTerminal(ARG_ARRAY_K, $2, NULL, 0, $3)};
	astPutChild($1, aux, 1);
	$$ = $1;
	}
	;

// 10
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

// 11
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
	
// 12
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

// 13
statement:
		 exp_decl	{$$ = $1;}
		 | composto_decl	{$$ = $1;}
		 | selecao_decl	{$$ = $1;}
		 | iteracao_decl	{$$ = $1;}
		 | retorno_decl	{$$ = $1;}
		;

// 14
exp_decl:
	exp ';'	{$$ = $1;}
	| ';'	{$$ = NULL;}
	;

// 15
selecao_decl:
	    IF '(' exp ')' statement else_stmt{
		$$ = astCreateNo(IF_K, NULL, NULL, 0);
		if($6){
			astNo* aux[] = {$3, $5, $6};
			astPutChild($$, aux, 3);
		}
		else{
			astNo* aux[] = {$3, $5};
			astPutChild($$, aux, 2);
		}
	}
	;

// 16
else_stmt:
	%prec IFX {
	$$ = NULL;
	}
	| ELSE statement{$$ = $2;}
	;

// 17
iteracao_decl:
	WHILE '(' exp ')' statement	{
	astNo* aux[] = {$3, $5};
	$$ = astCreateNo(WHILE_K, NULL, NULL, 0);
	astPutChild($$, aux, 2);

	}
	;

// 18
retorno_decl:
	RETURN ';'	{$$ = astCreateNo(RETURN_K, NULL, NULL, 0);}
	| RETURN exp ';'	{
	astNo* aux[] = {$2};
	$$ = astCreateNo(RETURN_K, NULL, NULL, 0);
	astPutChild($$, aux, 1);
	}
	;

// 19
exp:	
	var '=' exp	{
	astNo* aux[] = {$1, $3};
	$$ = astCreateNo(ASSIGN_K, NULL, NULL, 0);
	astPutChild($$, aux, 2);
	}
	| simple_exp	{$$ = $1;}
	;
	
// 20
var:
	ID	EPSLON_LINE{
	// AST
	$$ = astCreateTerminal(VAR_K, $1, NULL,0, $2);

	// Free
	free($1);
	
	}	
	| ID EPSLON_LINE '[' exp ']'	{
	astNo* aux[] = {$4};
	$$ = astCreateTerminal(VAR_ARRAY_K, $1, NULL, 0, $2);
	astPutChild($$, aux, 1);
	
	}
	;

// 21
simple_exp:
	soma_exp rel soma_exp	{
	astNo* aux[] = {$1, $3};
	astPutChild($2, aux, 2);
	$$ = $2;
	}
	| soma_exp	{$$ = $1;}
	;

// 22
rel:
 	LE	{$$ = astCreateNo(LEQ_K, NULL, NULL, 0);}
	|'<'	{$$ = astCreateNo(LESS_K, NULL, NULL, 0);}
	|'>'	{$$ = astCreateNo(GRAND_K, NULL, NULL, 0);}
	|GE	{$$ = astCreateNo(GEQ_K, NULL, NULL, 0);}
	|EQ	{$$ = astCreateNo(EQ_K, NULL, NULL, 0);}
	|DIFF	{$$ = astCreateNo(DIFF_K, NULL, NULL, 0);}
	;

// 23
soma_exp:
		soma_exp soma termo {
		astNo *aux[2] = {$1, $3};
		astPutChild($2, aux, 2);
		$$ = $2;
		}
	| termo	{$$ = $1;} ;

// 24
soma:
	'+'	{$$ = astCreateNo(PLUS_K, NULL, NULL, 0);}
	| '-'	{$$ = astCreateNo(MINUS_K, NULL, NULL, 0);};

// 25
termo:
	termo mult fator {
	astNo* aux[] = {$1, $3};
	astPutChild($2, aux, 2);
	$$ = $2;
	}
	| fator {$$ = $1;} ;

// 26
mult:
	'*'	{$$ = astCreateNo(MULT_K, NULL, NULL, 0);}
	| '/'	{$$ = astCreateNo(DIV_K, NULL, NULL, 0);}
	;

// 27
fator:
	 '(' exp ')'  {$$ = $2;}
	| var  {$$ = $1;}
	| act	{$$ = $1;}
	| NUM	EPSLON_LINE{
	char str[I2A_SZ];
	sprintf(str, "%d", $1);
	$$ = astCreateTerminal(NUM_K, strdup(str), NULL, 0, $2);
	}
	;

// 28
act:
   ID EPSLON_LINE '(' args ')' {
	// AST
	astNo* aux[] = {$4};
	$$ = astCreateTerminal(CALL_K, $1, NULL, 0, $2);
	astPutChild($$, aux, 1);

	// Free
	free($1);
	}
	;

// 29
args:
	arg_lista	{$$ = $1;}
	| /*epsilon*/	{$$ = NULL;}	
	;

// 30
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

	printf("Erro sintático : %s : linha %d\n", yytext, line_log);
	exit(0);
	return 1;
}

