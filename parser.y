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
%type <t_ast> programa decl_lista decl var_decl tipo_esp fun_decl params param_lista param composto_decl local_decl statement_lista statement exp_decl selecao_decl in_if iteracao_decl retorno_decl exp var simple_exp rel soma_exp soma termo mult fator act args arg_lista

%%

programa:			
		decl_lista{
		$$ = $1;
		ast_root = $1;
		}

decl_lista:
		  decl_lista decl	{
	$$ = createNo(kdecl_lista);
	childrenSpace($$,2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	}
		  | decl	{$$ = $1;}

decl:
	var_decl	{$$ = $1;}
	| fun_decl	{$$ = $1;}


var_decl:
		tipo_esp ID ';'		{
	$$ = createNo(kvar_decl);
	childrenSpace($$,1);
	$$->children[0] = $1;
	}
		| tipo_esp ID '[' NUM ']' ';'	{
	$$ = createNo(kvar_decl);
	childrenSpace($$,1);
	$$->children[0] = $1;
	}

tipo_esp:
		INT		{$$ = createNo(ktipo_esp);}
		| VOID		{$$ = createNo(ktipo_esp);}

fun_decl:
		tipo_esp ID '(' params ')' composto_decl	{
	$$ = createNo(kfun_decl);
	childrenSpace($$,3);
	$$->children[0] = $1;
	$$->children[1] = $4;
	$$->children[2] = $6;
	}

params:
	  param_lista 	{$$ = $1;}
	  | VOID	{$$ = createNo(kparams);}

param_lista:
		   param_lista ',' param	{
	$$ = createNo(kparam_lista);
	childrenSpace($$,2);
	$$->children[0] = $1;
	$$->children[1] = $3;
	}
		   | param	{$$ = $1;}
		   
param:
	 tipo_esp ID	{
	$$ = createNo(kparam);
	childrenSpace($$,1);
	$$->children[0] = $1;
	}
	 | tipo_esp ID '['']'	{
	$$ = createNo(kparam);
	childrenSpace($$,1);
	$$->children[0] = $1;
	}

composto_decl:
			'{' local_decl statement_lista '}'	{
	$$ = createNo(kcomposto_decl);
	childrenSpace($$,2);
	$$->children[0] = $2;
	$$->children[1] = $3;
	}

local_decl:
		  local_decl var_decl	{
	$$ = createNo(klocal_decl);
	childrenSpace($$,2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	}
		  | /*epsilon*/	{$$ = createNo(NIL);}

statement_lista:
			   statement_lista statement 	{
	$$ = createNo(kstatement_lista);
	childrenSpace($$,2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	}
			   | /*epsilon*/	{$$ = createNo(NIL);}
statement:
		 exp_decl	{$$ = $1;}
		 | composto_decl	{$$ = $1;}
		 | selecao_decl	{$$ = $1;}
		 | iteracao_decl	{$$ = $1;}
		 | retorno_decl	{$$ = $1;}

exp_decl:
		exp ';'	{$$ = $1;}
		| ';'	{$$ = createNo(kexp_decl);}

selecao_decl:
	IF '(' exp ')' in_if	{
	$$ = createNo(kselecao_decl);
	childrenSpace($$,2);
	$$->children[0] = $3; 
	$$->children[0] = $5;
	}

in_if:
	statement	{$$ = $1;}
	| ELSE statement	{
	$$ = createNo(kin_if);
	childrenSpace($$,1);
	$$->children[0] = $2;
	}


iteracao_decl:
			 WHILE '(' exp ')' statement	{
			 $$ = createNo(kiteracao_decl);
			 childrenSpace($$, 2);
			 $$->children[0] = $3;
			 $$->children[1] = $5;}

retorno_decl:
			RETURN ';'		{$$ = createNo(kretorno_decl);}
			| RETURN exp ';'	{
			$$ = createNo(kretorno_decl);
			childrenSpace($$, 1);
			$$->children[0] = $2;
			}

exp:
   var '=' exp	{
   $$ = createNo(kexp);
   childrenSpace($$, 2);
   $$->children[0] = $1;
   $$->children[1] = $3;
   }
   | simple_exp	{$$ = $1;}

var:
   ID	{$$ = createNo(kvar);} 
   | ID '[' exp ']'	
   {
   $$ = createNo(kvar);
   childrenSpace($$, 1);
   $$->children[0] = $3;
   }

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
	 '(' exp ')' {$$ = createNo(kfact);}
	 | var  {$$ = $1;}
	 | act	{$$ = $1;}
	 | NUM	{$$ = createNo(kfact);}

act:
   ID '(' args ')'	{
	$$ = createNo(kact);
	childrenSpace($$, 1);
	$$->children[0] = $3;
	}

args:
	arg_lista	{$$ = $1;}
	| /*epsilon*/	{$$ = createNo(NIL);}

arg_lista:
		 arg_lista ',' exp{
		 createNo(karg_lista);
		 childrenSpace($$, 2);
		 $$->children[0] = $1;
		 $$->children[1] = $3;
		 }
		 | exp	{$$ = $1;}

%%

void yyerror(char* err){
	printf(ERR_SYN, yylval.token, lineno);
}

int yywrap(){
	return 1;
}

int main(){
	yyparse();
	printTree(ast_root, 0);
	freeTree(ast_root);
	return 1;
}

