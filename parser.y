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
	childrenSpace($$,3);
	$$->children[0] = $1;
	$$->children[1] = createNo(terminal);
	$$->children[2] = createNo(terminal);
	}
		| tipo_esp ID '[' NUM ']' ';'	{
	$$ = createNo(kvar_decl);
	childrenSpace($$,6);
	$$->children[0] = $1;
	$$->children[1] = createNo(terminal);
	$$->children[2] = createNo(terminal);
	$$->children[3] = createNo(terminal);
	$$->children[4] = createNo(terminal);
	$$->children[5] = createNo(terminal);
	}

tipo_esp:
		INT		{
		$$ = createNo(ktipo_esp);
		childrenSpace($$, 1);
		$$->children[0] = createNo(terminal);
		}
		| VOID		{
		$$ = createNo(ktipo_esp);
		childrenSpace($$, 1);
		$$->children[0] = createNo(terminal);
		}

fun_decl:
		tipo_esp ID '(' params ')' composto_decl	{
	$$ = createNo(kfun_decl);
	childrenSpace($$,6);
	$$->children[0] = $1;
	$$->children[1] = createNo(terminal);
	$$->children[2] = createNo(terminal);
	$$->children[3] = $4;
	$$->children[4] = createNo(terminal);
	$$->children[5] = $6;
	}

params:
	  param_lista 	{$$ = $1;}
	  | VOID	{
	  $$ = createNo(kparams);
	  childrenSpace($$, 1);
	  $$->children[0] = createNo(terminal);
	  }

param_lista:
		   param_lista ',' param	{
	$$ = createNo(kparam_lista);
	childrenSpace($$,3);
	$$->children[0] = $1;
	$$->children[1] = createNo(terminal);
	$$->children[2] = $3;
	}
		   | param	{$$ = $1;}
		   
param:
	 tipo_esp ID	{
	$$ = createNo(kparam);
	childrenSpace($$,2);
	$$->children[0] = $1;
	$$->children[1] = createNo(terminal);
	}
	 | tipo_esp ID '['']'	{
	$$ = createNo(kparam);
	childrenSpace($$,3);
	$$->children[0] = $1;
	$$->children[1] = createNo(terminal);
	$$->children[2] = createNo(terminal);
	}

composto_decl:
			'{' local_decl statement_lista '}'	{
	$$ = createNo(kcomposto_decl);
	childrenSpace($$,4);
	$$->children[0] = createNo(terminal);
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = createNo(terminal);
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
		exp ';'	{
		$$ = createNo(kexp_decl);
		childrenSpace($$, 2);
		$$->children[0] = $1;
		$$->children[1] = createNo(terminal);
		}
		| ';'	{
		$$ = createNo(kexp_decl);
		childrenSpace($$, 1);
		$$->children[0] = createNo(terminal);
		}

selecao_decl:
	IF '(' exp ')' in_if	{
	$$ = createNo(kselecao_decl);
	childrenSpace($$,5);
	 $$->children[0] = createNo(terminal);
	 $$->children[1] = createNo(terminal);
	$$->children[2] = $3; 
	 $$->children[3] = createNo(terminal);
	$$->children[4] = $5;
	}

in_if:
	statement	{$$ = $1;}
	| ELSE statement	{
	$$ = createNo(kin_if);
	childrenSpace($$,2);
	$$->children[0] = createNo(terminal);
	$$->children[1] = $2;
	}


iteracao_decl:
			 WHILE '(' exp ')' statement	{
			 $$ = createNo(kiteracao_decl);
			 childrenSpace($$, 5);
			 $$->children[0] = createNo(terminal);
			 $$->children[1] = createNo(terminal);
			 $$->children[2] = $3;
			 $$->children[3] = createNo(terminal);
			 $$->children[4] = $5;
			 }

retorno_decl:
			RETURN ';'		{
			$$ = createNo(kretorno_decl);
			childrenSpace($$, 2);
			$$->children[0] = createNo(terminal);
			$$->children[1] = createNo(terminal);
			}
			| RETURN exp ';'	{
			$$ = createNo(kretorno_decl);
			childrenSpace($$, 3);
			$$->children[0] = createNo(terminal);
			$$->children[1] = $2;
			$$->children[2] = createNo(terminal);
			}

exp:
   var '=' exp	{
   $$ = createNo(kexp);
   childrenSpace($$, 3);
   $$->children[0] = $1;
   $$->children[1] = createNo(terminal);
   $$->children[2] = $3;
   }
   | simple_exp	{$$ = $1;}

var:
   ID	{
   $$ = createNo(kvar);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
	} 
   | ID '[' exp ']'	
   {
   $$ = createNo(kvar);
   childrenSpace($$, 4);
   $$->children[0] = createNo(terminal);
   $$->children[1] = createNo(terminal);
   $$->children[2] = $3;
   $$->children[3] = createNo(terminal);
   }

simple_exp:
		  soma_exp rel soma_exp	{
		  $$ = createNo(ksimple_exp);
		  childrenSpace($$, 3);
		  $$->children[0] = $1;
		  $$->children[1] = $2;
		  $$->children[2] = $3;
		  }
		  | soma_exp	{$$ = $1;}

rel:
   LE	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
   }
   |'<'	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
   }
   |'>'	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
   }
   |GE	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
   }
   |EQ	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
   }
   |DIFF {
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
   }


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
	'+'		{
	$$ = createNo(ksum);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
	}
	| '-'	{
	$$ = createNo(ksum);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
	}


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
	'*'		{
	$$ = createNo(kmult);
	childrenSpace($$, 1);
	$$->children[0] = createNo(terminal);
	}
	| '/'	{
	$$ = createNo(kmult);
	childrenSpace($$, 1);	
	$$->children[0] = createNo(terminal);
	}

fator:
	 '(' exp ')' {
	 $$ = createNo(kfact);
	 childrenSpace($$, 3);
	 $$->children[0] = createNo(terminal);
	 $$->children[1] = $2;
	 $$->children[2] = createNo(terminal);
	 }
	 | var  {$$ = $1;}
	 | act	{$$ = $1;}
	 | NUM	{$$ = createNo(terminal);}

act:
   ID '(' args ')'	{
	$$ = createNo(kact);
	childrenSpace($$, 4);
	$$->children[0] = createNo(terminal);
	$$->children[1] = createNo(terminal);
	$$->children[2] = $3;
	$$->children[3] = createNo(terminal);
	}

args:
	arg_lista	{$$ = $1;}
	| /*epsilon*/	{$$ = createNo(NIL);}

arg_lista:
		 arg_lista ',' exp{
		 createNo(karg_lista);
		 childrenSpace($$, 3);
		 $$->children[0] = $1;
		 $$->children[1] = createNo(terminal);
		 $$->children[2] = $3;
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

