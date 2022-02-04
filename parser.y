%{
#include <stdio.h>
#include <string.h>
#include "symtab/symtab.h"
#include "defines.h"
#include "ast.h"
#define MAXDIGIT 100

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
%type <ast_type> termo fator mult rel soma soma_exp simple_exp var exp retorno_decl act iteracao_decl in_if selecao_decl args arg_lista
%type <ast_type> exp_decl statement statement_lista local_decl composto_decl param param_lista params fun_decl tipo_esp var_decl decl decl_lista
%%

programa:	
		decl_lista{printf("Program matched\n");
					ast_root = $1;}

decl_lista:
		  decl_lista decl	{
		  	$$ = storeNo("decl_lista", NULL, 0);
			addChild($$, $1);
			addChild($$, $2);
		  }
		  | decl	{$$ = $1;}

decl:
	var_decl	{$$ = $1;}
	| fun_decl	{$$ = $1;}

var_decl:
		tipo_esp ID ';'		{
							ast aux[2];
							$$ = storeNo("var_decl", NULL, 0);
							aux[0] = storeNo($2, NULL, 0);
							aux[1] = storeNo(";", NULL, 0);

							addChild($$, $1);
							addChild($$, aux[0]);
							addChild($$, aux[1]);
							}	
		| tipo_esp ID '[' NUM ']' ';'	{
										char nval[MAXDIGIT];
										ast aux[5];
										
										sprintf(nval, "%d", $4);	// Convert int to alpha

										$$ = storeNo("var_decl", NULL, 0);
										aux[0] = storeNo($2, NULL, 0);
										aux[1] = storeNo("[", NULL, 0);
										aux[2] = storeNo(nval, NULL, 0);
										aux[3] = storeNo("]", NULL, 0);
										aux[4] = storeNo(";", NULL, 0);

										addChild($$, $1);
										addChild($$, aux[0]);
										addChild($$, aux[1]);
										addChild($$, aux[2]);
										addChild($$, aux[3]);
										addChild($$, aux[4]);
										}

tipo_esp:
		INT		{$$ = storeNo("INT", NULL, 0);}	
		| VOID	{$$ = storeNo("VOID", NULL, 0);}

fun_decl:
		tipo_esp ID '(' params ')' composto_decl	{
		ast aux[3];
		$$ = storeNo("fun_decl", NULL, 0);
		aux[0] = storeNo($2, NULL, 0);
		aux[1] = storeNo("(", NULL, 0);
		aux[2] = storeNo(")", NULL, 0);
		addChild($$, $1);
		addChild($$, aux[0]);
		addChild($$, aux[1]);
		addChild($$, $4);
		addChild($$, aux[2]);
		addChild($$, $6);
		}

params:
	  param_lista 	{$$ = $1;}
	  | VOID		{$$ = storeNo("VOID", NULL, 0);}

param_lista:
		   param_lista ',' param	{
									ast aux;
									$$ = storeNo("param_lista", NULL, 0);
									aux = storeNo(",", NULL, 0);
									addChild($$, $1);
									addChild($$, aux);
									addChild($$, $3);
							   		}
		   | param		{$$ = $1;}
		   
param:
	 tipo_esp ID
	 | tipo_esp ID '['']'	{
	 						ast aux[3];
							$$ = storeNo("param", NULL, 0);
							aux[0] = storeNo($2, NULL, 0);
							aux[1] = storeNo("[", NULL, 0);
							aux[2] = storeNo("]", NULL, 0);
							addChild($$, $1);
							addChild($$, aux[0]);
							addChild($$, aux[1]);
							addChild($$, aux[2]);
							}


composto_decl:
			'{' local_decl statement_lista '}'	{
												ast aux[2];
												$$ = storeNo("composto_decl", NULL, 0);
												aux[0] = storeNo("{", NULL, 0);
												aux[1] = storeNo("}", NULL, 0);
												addChild($$, aux[0]);
												addChild($$, $2);
												addChild($$, $3);
												addChild($$, aux[1]);
												}

local_decl:
		  local_decl var_decl	{$$ = $1;
		  						children_concat($$,$1);
								}
		  | /*epsilon*/	{$$ = storeNo("local_decl", NULL, 0);}

statement_lista:
			   statement_lista statement {$$ = $1;
			   							 children_concat($$, $1);
										 }
			   | /*epsilon*/	{$$ = storeNo("statement_lista", NULL, 0);}	

statement:
		 exp_decl	{$$ = $1;}
		 | composto_decl	{$$ = $1;}
		 | selecao_decl		{$$ = $1;}
		 | iteracao_decl	{$$ = $1;}
		 | retorno_decl		{$$ = $1;}

exp_decl:
		exp ';'	{
				ast aux;
				$$ = storeNo("exp", NULL, 0);
				aux = storeNo(";", NULL, 0);
				addChild($$, aux);
				}
		| ';'	{$$ = storeNo(";", NULL, 0);}

selecao_decl:
	IF '(' exp ')' in_if	{
							ast aux[3];
							$$ = storeNo("selecao_decl", NULL, 0);
							aux[0] = storeNo("IF", NULL, 0);
							aux[1] = storeNo("(", NULL, 0);
							aux[2] = storeNo(")", NULL, 0);
							addChild($$, aux[0]);
							addChild($$, aux[1]);
							addChild($$, $3);
							addChild($$, aux[2]);
							addChild($$, $5);
							}

in_if:
	statement	{$$ = $1;}
	| ELSE statement	{ast aux;
						$$ = storeNo("in_if", NULL, 0);
						aux = storeNo("ELSE", NULL, 0);
						addChild($$, aux);
						addChild($$, $2);
						}


iteracao_decl:
			 WHILE '(' exp ')' statement {
			 								ast aux[3];
											$$ = storeNo("interacao_decl", NULL, 0);
											aux[0] = storeNo("WHILE", NULL, 0);
											aux[1] = storeNo("(", NULL, 0);
											aux[2] = storeNo(")", NULL, 0);
											addChild($$, aux[0]);
											addChild($$, aux[1]);
											addChild($$, $3);
											addChild($$, aux[2]);
											addChild($$, $5);
											}

retorno_decl:
			RETURN ';'		{
							ast aux[2];
							aux[0] = storeNo("RETURN", NULL, 0);
							aux[1] = storeNo(";", NULL, 0);
							$$ = storeNo("retorno_decl", aux, 2);
							}
			| RETURN exp ';'{
							ast aux[2];
							$$ = storeNo("retorno_decl", NULL, 0);
							aux[0] = storeNo("RETURN", NULL, 0);
							aux[1] = storeNo(";", NULL, 0);
							addChild($$, aux[0]);
							addChild($$, $2);
							addChild($$, aux[1]);
							}

exp:
   var '=' exp	{
   				
   				$$ = storeNo("=", NULL, 0);
				addChild($$, $1);
				addChild($$, $3);
				}
   | simple_exp {$$ = $1;}

var:
   ID	{$$ = storeNo($1, NULL, 0);}
   | ID '[' exp ']'		{
						ast aux[3];
   						$$ = storeNo("var", NULL, 0);
   						aux[0] = storeNo($1, NULL, 0);
						aux[1] = storeNo("[", NULL, 0);
						aux[2] = storeNo("]", NULL, 0);
						addChild($$, aux[0]);
						addChild($$, aux[1]);
						addChild($$, $3);
						addChild($$, aux[2]);
						}

simple_exp:
		  soma_exp rel soma_exp	{
			addChild($2, $1);
			addChild($2,$3);
			$$ = $2;
			ast_root = $2;
			}	
		  | soma_exp	{$$ = $1;}

rel:
   LE	 {$$ = storeNo("<=", NULL, 0);}
   |'<'	 {$$ = storeNo("<", NULL, 0);}
   |'>'  {$$ = storeNo(">", NULL, 0);}
   |GE   {$$ = storeNo(">=", NULL, 0);}
   |EQ	 {$$ = storeNo("==", NULL, 0);}
   |DIFF {$$ = storeNo("!=", NULL, 0);}

soma_exp:
		soma_exp soma termo	{
			addChild($2, $1);
			addChild($2, $3);
			$$ = $2;
		} 
		| termo {$$ = $1;}

soma:
	'+' 	{$$ = storeNo("+", NULL, 0);}	
	| '-'	{$$ = storeNo("-", NULL, 0);}


termo:
	 termo mult fator{
		addChild($2, $1);
		addChild($2, $3);
	 	
		$$ = $2;
//		ast_root = $2;
	} 
	| fator {
		$$ = $1;
	}


mult:
	'*'	{$$ = storeNo("*", NULL, 0);}
	| '/'{$$ = storeNo("/", NULL, 0);}

fator:
	 '(' exp ')'{
	 			ast aux[2];
	 			$$ = storeNo("fator", NULL, 0);
				aux[0] = storeNo("(", NULL, 0);
				aux[1] = storeNo(")", NULL, 0);
				addChild($$, aux[0]);
				addChild($$, $2);
				addChild($$, aux[1]);
				}
	 | var	{$$ = $1;}
	 | act	{$$ = $1;}
	 | NUM	{
	 char nval[MAXDIGIT];
	 sprintf(nval, "%d", $1);
	 $$ = storeNo(nval, NULL, 0);}

act:
   ID '(' args ')'	{
   					ast aux[3];
					$$ = storeNo("act", NULL, 0);
					aux[0] = storeNo($1, NULL, 0);
					aux[1] = storeNo("(", NULL, 0);
					aux[2] = storeNo(")", NULL, 0);
					addChild($$, aux[0]);
					addChild($$, aux[1]);
					addChild($$, $3);
					addChild($$, aux[2]);
					}

args:
	arg_lista	{$$ = $1;}
	| /*epislon*/ {$$ = storeNo("args", NULL, 0);}

arg_lista:
		 arg_lista ',' exp	{
		 					ast aux;
							$$ = storeNo("arg_lista", NULL, 0);
							aux = storeNo(",", NULL, 0);
		 					addChild($$, $1);
							addChild($$, aux);
							addChild($$, $3);}
		 | exp	{$$ = $1;}

%%

void yyerror(char* err){
	printf(ERR_SYN, yylval.token, lineno);
}

int yywrap(){
	printf("Abstract syntax tree:\n");
	preorderWalk(ast_root, 0);
	freeA(ast_root);
	return 1;
}

int main(){
	yyparse();
	return 1;
}

