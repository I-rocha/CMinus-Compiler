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
//
// KEYWORDS
%token IF ELSE INT RETURN VOID WHILE LE GE EQ DIFF

// OP -> Default declaration

// ID
%token <s>ID

// NUM
%token <val>NUM

// Terminal types
%type <val>simple_exp <val>soma_exp <val>soma <val>termo <val>mult <val>fator <val>rel
%type <symb>tip_esp <symb> var_decl
%%

//TODO: Remover esse regex
tmp:
  simple_exp {printf("##PARSER-EXP: %d\n", $1);}
  | var_decl {
  				symbol no;
				add("main", $1->type, $1->name, 235);
  				
				if($1->name == NULL){printf("NULL\n");}
				no=get("main",$1->name);
  				if($1->type != NULL) free($1->type);
  				if($1->name != NULL) free($1->name);
  				if($1 != NULL) free($1);
				printf("##PARSER-DECL: %s\t##PARSER-SCOPE: %s\t##PARSER-VAL: %d\n", no->name, no->escopo, no->ival);
			}

var_decl:
		tip_esp ID ';'	{$$ = $1; $$->name = strdup($2);}
		| tip_esp ID '[' NUM ']' ';'	{$$ = $1; $$->name = strdup($2);/*TODO: Alterar para armazenar vetor*/}

tip_esp:
		INT	{
		if($$ == NULL)	
			$$ = (struct symbol*)malloc(sizeof(struct symbol));
		$$->type = strdup("int");
		}
		
		| VOID	{
		if($$ == NULL)	
			$$ = (struct symbol*)malloc(sizeof(struct symbol));
		$$->type = strdup("void");
		}

simple_exp:
		  soma_exp rel soma_exp	{
		 	switch($2){
				// <=
				case 0:
					$$ = ($1 <= $3)?1:0;
					break;

				// <
				case 1:		
					$$ = ($1 < $3)?1:0;
					break;

				// >
				case 2:
					$$ = ($1 > $3)?1:0;
					break;

				// >=
				case 3:
					$$ = ($1 >= $3)?1:0;
					break;

				// ==
				case 4:
					$$ = ($1 == $3)?1:0;
					break;

				// !=
				case 5:
					$$ = ($1 != $3)?1:0;
					break;
			}
		  }
		  | soma_exp

rel:
   LE	{$$ = 0;}
   |'<'	{$$ = 1;}
   |'>'	{$$ = 2;}
   |GE	{$$ = 3;}
   |EQ	{$$ = 4;}
   |DIFF	{$$ = 5;}

soma_exp:
		soma_exp soma termo {
		if($2 == 0)
			$$=$1+$3;
		else 
			$$=$1-$3; 
		
		/*TODO: Remover essa linha*/
		printf("##Parser-val: %d\n", $$);
		}
		| termo 
		;

soma:
	'+' 	{$$ = 0;}
	| '-'	{$$ = 1;}
	;


termo:
	 termo mult fator {
	 if($2==0)
	 	$$ = $1*$3;
	 else
	 	$$ = $1/$3;
	}
	| fator 
	;

mult:
	'*'	{$$ = 0;}
	| '/' {$$ = 1;}
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

