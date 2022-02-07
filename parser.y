%{
#include <stdio.h>
#include <string.h>
#include "symtab/symtab.h"
#include "defines.h"
#include "ast.h"
#include "semantic.h"

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
%type <t_ast> programa decl_lista decl var_decl tipo_esp fun_decl params param_lista param composto_decl local_decl statement_lista statement exp_decl selecao_decl iteracao_decl retorno_decl exp var simple_exp rel soma_exp soma termo mult fator act args arg_lista

%nonassoc IFX
%nonassoc ELSE

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
	$$->children[1] = createNoTerminal(kID);
	$$->children[2] = createNoTerminal(kop);

	// AST terminais
	$$->children[1]->name = strdup($2);
	$$->children[2]->name = strdup(";");
	}
		| tipo_esp ID '[' NUM ']' ';'	{
	$$ = createNo(kvar_decl);
	childrenSpace($$,6);
	$$->children[0] = $1;
	$$->children[1] = createNoTerminal(kID);
	$$->children[2] = createNoTerminal(kop);
	$$->children[3] = createNoTerminal(kNUM);
	$$->children[4] = createNoTerminal(kop);
	$$->children[5] = createNoTerminal(kop);

	// AST terminais
	$$->children[1]->name = strdup($2);
	$$->children[2]->name = strdup("[");
	$$->children[3]->val = $4;
	$$->children[4]->name = strdup("]");
	$$->children[5]->name = strdup(";");
	}

tipo_esp:
		INT		{
		$$ = createNo(ktipo_esp);
		childrenSpace($$, 1);
		$$->children[0] = createNoTerminal(kint);
		$$->children[0]->name = strdup("int");	// AST terminal
		}
		| VOID		{
		$$ = createNo(ktipo_esp);
		childrenSpace($$, 1);
		$$->children[0] = createNoTerminal(kvoid);
		$$->children[0]->name = strdup("void");	// AST terminal
		}

fun_decl:
		tipo_esp ID '(' params ')' composto_decl	{
	$$ = createNo(kfun_decl);
	childrenSpace($$,6);
	$$->children[0] = $1;
	$$->children[1] = createNoTerminal(kID);
	$$->children[2] = createNoTerminal(kop);
	$$->children[3] = $4;
	$$->children[4] = createNoTerminal(kop);
	$$->children[5] = $6;

	// AST terminais
	$$->children[1]->name = strdup($2);
	$$->children[2]->name = strdup("(");
	$$->children[4]->name = strdup(")");
	}

params:
	  param_lista 	{$$ = $1;}
	  | VOID	{
	  $$ = createNo(kparams);
	  childrenSpace($$, 1);
	  $$->children[0] = createNoTerminal(kvoid);
	  $$->children[0]->name = strdup("void");	// AST terminal
	  }

param_lista:
		   param_lista ',' param	{
	$$ = createNo(kparam_lista);
	childrenSpace($$,3);
	$$->children[0] = $1;
	$$->children[1] = createNoTerminal(kop);
	$$->children[2] = $3;

	$$->children[1]->name = strdup(",");	// AST terminal
	}
		   | param	{$$ = createNo(kparam_lista); childrenSpace($$, 1); $$->children[0] = $1;}
		   
param:
	 tipo_esp ID	{
	$$ = createNo(kparam);
	childrenSpace($$,2);
	$$->children[0] = $1;
	$$->children[1] = createNoTerminal(kID);
	$$->children[1]->name = strdup($2);	// AST terminal
	}
	 | tipo_esp ID '['']'	{
	$$ = createNo(kparam);
	childrenSpace($$,3);
	$$->children[0] = $1;
	$$->children[1] = createNoTerminal(kID);
	$$->children[2] = createNoTerminal(kop);
	$$->children[3] = createNoTerminal(kop);

	// AST terminal
	$$->children[1]->name = strdup($2);
	$$->children[2]->name = strdup("[");
	$$->children[3]->name = strdup("]");
	}

composto_decl:
			'{' local_decl statement_lista '}'	{
	$$ = createNo(kcomposto_decl);
	childrenSpace($$,4);
	$$->children[0] = createNoTerminal(kop);
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = createNoTerminal(kop);

	// AST terminais
	$$->children[0]->name = strdup("{");
	$$->children[3]->name = strdup("}");
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
		$$->children[1] = createNoTerminal(kop);
		$$->children[1]->name = strdup(";");	// AST terminal
		}
		| ';'	{
		$$ = createNo(kexp_decl);
		childrenSpace($$, 1);
		$$->children[0] = createNoTerminal(kop);
		$$->children[0]->name = strdup(";");	// AST terminal
		}

selecao_decl:
	IF '(' exp ')' statement %prec IFX{
	$$ = createNo(kselecao_decl);
	childrenSpace($$,5);
	$$->children[0] = createNoTerminal(kif);
	$$->children[1] = createNoTerminal(kop);
	$$->children[2] = $3; 
	$$->children[3] = createNoTerminal(kop);
	$$->children[4] = $5;

	// AST terminais
	$$->children[0]->name = strdup("if");
	$$->children[1]->name = strdup("(");
	$$->children[3]->name = strdup(")");
	}
	| IF '(' exp ')' statement ELSE statement{
		$$ = createNo(kselecao_decl);
	childrenSpace($$,7);
	$$->children[0] = createNoTerminal(kif);
	$$->children[1] = createNoTerminal(kop);
	$$->children[2] = $3; 
	$$->children[3] = createNoTerminal(kop);
	$$->children[4] = $5;
	$$->children[5] = createNoTerminal(kelse);
	$$->children[6] = $7;

	// AST terminais
	$$->children[0]->name = strdup("if");
	$$->children[1]->name = strdup("(");
	$$->children[3]->name = strdup(")");
	$$->children[5]->name = strdup("else");
	}
/*
in_if:
	statement	{$$ = $1;}
	| ELSE statement	{
	$$ = createNo(kin_if);
	childrenSpace($$,2);
	$$->children[0] = createNo(terminal);
	$$->children[1] = $2;
	}
*/

iteracao_decl:
			 WHILE '(' exp ')' statement	{
			 $$ = createNo(kiteracao_decl);
			 childrenSpace($$, 5);
			 $$->children[0] = createNoTerminal(kwhile);
			 $$->children[1] = createNoTerminal(kop);
			 $$->children[2] = $3;
			 $$->children[3] = createNoTerminal(kop);
			 $$->children[4] = $5;

			 // AST terminais
			 $$->children[0]->name = strdup("while");
			 $$->children[1]->name = strdup("(");
			 $$->children[3]->name = strdup(")");
			 }

retorno_decl:
			RETURN ';'		{
			$$ = createNo(kretorno_decl);
			childrenSpace($$, 2);
			$$->children[0] = createNoTerminal(kreturn);
			$$->children[1] = createNoTerminal(kop);
			
			// AST terminais
			$$->children[0]->name = strdup("return");
			$$->children[1]->name = strdup(";");
			}
			| RETURN exp ';'	{
			$$ = createNo(kretorno_decl);
			childrenSpace($$, 3);
			$$->children[0] = createNoTerminal(kreturn);
			$$->children[1] = $2;
			$$->children[2] = createNoTerminal(kop);
				
			// AST terminais
			$$->children[0]->name = strdup("return");
			$$->children[2]->name = strdup(";");
		}

exp:
   var '=' exp	{
   $$ = createNo(kexp);
   childrenSpace($$, 3);
   $$->children[0] = $1;
   $$->children[1] = createNoTerminal(kop);
   $$->children[2] = $3;

   $$->children[1]->name = strdup("=");	// AST terminal
   }
   | simple_exp	{$$ = $1;}

var:
   ID	{
   $$ = createNo(kvar);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kID);
	$$->children[0]->name = strdup($1);	// AST terminal
	} 
   | ID '[' exp ']'	
   {
   $$ = createNo(kvar);
   childrenSpace($$, 4);
   $$->children[0] = createNoTerminal(kID);
   $$->children[1] = createNoTerminal(kop);
   $$->children[2] = $3;
   $$->children[3] = createNoTerminal(kop);

	// AST terminais
   $$->children[0]->name = strdup($1);
   $$->children[1]->name = strdup("[");
   $$->children[3]->name = strdup("]");
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
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("<=");	// AST terminal
   }
   |'<'	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("<");	// AST terminal
   }
   |'>'	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup(">");	// AST terminal
   }
   |GE	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup(">=");	//AST terminal
   }
   |EQ	{
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("==");	//AST terminal
   }
   |DIFF {
   $$ = createNo(krel);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("!=");	//AST terminal
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
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("+");	//AST terminal
	}
	| '-'	{
	$$ = createNo(ksum);
	childrenSpace($$, 1);
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("-");	// AST terminal
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
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("*");
	}
	| '/'	{
	$$ = createNo(kmult);
	childrenSpace($$, 1);	
	$$->children[0] = createNoTerminal(kop);
	$$->children[0]->name = strdup("/");//AST Terminal
	}

fator:
	 '(' exp ')' {
	 $$ = createNo(kfact);
	 childrenSpace($$, 3);
	 $$->children[0] = createNoTerminal(kop);
	 $$->children[1] = $2;
	 $$->children[2] = createNoTerminal(kop);

	//AST terminais
	 $$->children[0]->name = strdup("(") ;
	 $$->children[2]->name = strdup(")");
	 }
	 | var  {$$ = $1;}
	 | act	{$$ = $1;}
	 | NUM	{
	 $$ = createNoTerminal(kNUM);
	 $$->val = $1;	//AST terminal
	 }

act:
   ID '(' args ')'	{
	$$ = createNo(kact);
	childrenSpace($$, 4);
	$$->children[0] = createNoTerminal(kID);
	$$->children[1] = createNoTerminal(kop);
	$$->children[2] = $3;
	$$->children[3] = createNoTerminal(kop);

	//AST terminais
	$$->children[0]->name = strdup($1);
	$$->children[1]->name = strdup("(");
	$$->children[3]->name = strdup(")");
	}

args:
	arg_lista	{$$ = $1;}
	| /*epsilon*/	{$$ = createNo(NIL);}

arg_lista:
		 arg_lista ',' exp{
		$$ = createNo(karg_lista);
		 childrenSpace($$, 3);
		 $$->children[0] = $1;
		 $$->children[1] = createNoTerminal(kop);
		 $$->children[2] = $3;
		 
		 // AST terminais
		 $$->children[1]->name = strdup(",");
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
	table(ast_root);
	freeTree(ast_root);
	return 1;
}

