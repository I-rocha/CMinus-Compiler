#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variadic
#include <stdarg.h>

#include "symtab.h"

// TOKEN
#include "../ast/ast.h"

/*	PRIVATE HEADER	*/

/* Assign all field to entry */
void assignState(
		symEntry* entry, 
		Token id, 
		char* lexeme, 
		Token type, 
		unsigned short attr, 
		Token* param, 
		unsigned short* call, 
		unsigned short* def, 
		unsigned short ndef,
		unsigned short ncall,
		unsigned short nparam,
		symTable* env, 
		int null, 
		symEntry* prox
		);

/* Assign main data to entry */
void assignEntry(symEntry* entry, Token id, char* lexeme, Token type, unsigned short attr, symTable* env);

/* Assign default struct to entry */
void assignDefault(symEntry* entry);

symTable* symTInit(){
	symTable* env;

	env = (symTable*) malloc(sizeof(symTable));

	env->table = (symEntry*)malloc(NSYMHASH * sizeof(symEntry));
	env->child = NULL;
	env->parent = env;

	for(int i = 0; i < NSYMHASH; i++){
		assignDefault(&env->table[i]);
		env->table[i].env = env;
	}

	env->len = 0;
	env->scope = strdup("GLOBAL");
	return env;
}

void assignState(
		symEntry* entry, 
		Token id, 
		char* lexeme, 
		Token type, 
		unsigned short attr, 
		Token* param, 
		unsigned short* call, 
		unsigned short* def, 
		unsigned short ndef,
		unsigned short ncall,
		unsigned short nparam,
		symTable* env, 
		int null, 
		symEntry* prox
		){
        entry->id = id;
        entry->lexeme = (lexeme) ? strdup(lexeme) : NULL; 
        entry->type = type;
	entry->attr = attr;
	entry->param = param;
        entry->call = call;
        entry->def = def;
	entry->ndef = ndef,
	entry->ncall = ncall,
	entry->nparam = nparam,
        entry->env = env;
        entry->null = null; 
        entry->prox = prox; 
	return;
}

void assignEntry(symEntry* entry, Token id, char* lexeme, Token type, unsigned short attr, symTable* env){
         entry->id = id;
         entry->lexeme = (lexeme) ? strdup(lexeme) : NULL; 
         entry->type = type;
         entry->attr = attr;
	 entry->env = env;
	 entry->null = 0;
	 return;
}

void assignDefault(symEntry* entry){
	assignState(entry, BLANK, NULL, BLANK, 0, NULL, NULL, NULL, 0, 0, 0, NULL, 1, NULL);
	return;
}

/*
int symTFLL(symEntry* no){
	symEntry *aux, *auxprev;

	if(no){
		free(no->lexeme);
	}
	aux = no->prox;

	while(aux){
		auxprev = aux;
		aux = aux->prox;
		free(auxprev->lexeme);
		free(auxprev);
	}

	return 1;
}

int symTDeepFree(symTable* hash){
	if(!hash)
		return 0;
	
	for(int i = 0; i < hash->len; i++){
		symTDeepFree(&hash->child[i]);
		free(hash->child);
	}

	for(int i = 0; i < NSYMHASH; i++){
		symTFLL(&hash->table[i]);
	}

	return 1;
}

int symTFree(symTable* hash){
	if(!symTDeepFree(hash))
		return 0;

	if(hash)
		free(hash);
	return 1;
}*/

int symTKey(symEntry* item){
	int key = 0;
	int sz = 0;

	if(item->lexeme == NULL)
		return 0;

	sz = strlen(item->lexeme);

	while(sz > 0){
		key += (int)item->lexeme[sz-1];
		sz--;
	}

	key = key % NSYMHASH;
	return key;
}

int symTNameEquals(char* n1, char* n2){
	if((n1 && !n2) || (!n1 && n2))
		return -1;

	// If name is different
	if((n1 && n2) && (strcmp(n1, n2) != 0))
		return -1;

	return 1;

}

symEntry* putLocal(symTable* hash, Token tok, char* lexeme, Token type, int attr, unsigned short line){
	symEntry toPut, *aux;
	int key = -1;

	if(!hash || !hash->table)
		return NULL;

	assignDefault(&toPut);					// Default state
	assignEntry(&toPut, tok, lexeme, type, attr, hash);	// Add main info
	key = symTKey(&toPut);

	aux = &hash->table[key];

	if(aux->null == 1){
		*aux = toPut;
		return aux;
	}

	while(aux->prox){

		/* Already Exists */
		if(symTNameEquals(aux->lexeme, toPut.lexeme) == 1){
			return aux;
		}

		/* Keep looking */
		aux = aux->prox;
	}

	/* Already Exists */
	if(symTNameEquals(aux->lexeme, toPut.lexeme) == 1){
		return aux;
	}

	aux->prox = (symEntry*)malloc(sizeof(symEntry));
	*(aux->prox) = toPut;
	return aux;
}

symEntry* putGlobal(symTable* hash, Token tok, char* lexeme, Token type, int attr, unsigned short line){
	symTable* last_h;
	symEntry* entry;

	last_h = hash;

	if(!hash)
		return NULL;
	
	while(hash != last_h){
		entry = putLocal(hash, tok, lexeme, type, attr, line);

		if(entry)
			return entry;

		last_h = hash;
		hash = hash->parent;

	}

	return NULL;
}

symEntry* symTPut(symTable* hash, Token tok, char* lexeme, Token type, int attr, unsigned short line, int ID_CATEGORY){
	symEntry* entry;

	switch(ID_CATEGORY){
		case _DECLARATION:
			entry = putLocal(
					hash,
					tok,
					lexeme,
					type,
					attr,
					line
					);
			updateDef(entry, line);
			break;
		case _CALL:
			entry = putGlobal(
					hash,
					tok,
					lexeme,
					type,
					attr,
					line
					);
			symTUpdateCall(entry, line);
			break;
		default:
			return NULL;
	}
	return entry;
}

symEntry* symTLook(symTable* hash, char* lookup){
	symTable* last_table = NULL;
	symEntry toPut, *aux;
	int key = -1;

	if(!hash || !hash->table)
		return NULL;

	assignDefault(&toPut);					// Default state
	assignEntry(&toPut, BLANK, lookup, BLANK, 0, NULL);	// Add main info
	key = symTKey(&toPut);

	// Look over parent
	while(last_table != hash){
		aux = &hash->table[key];	

		if(aux->null == 1){
			last_table = hash;
			hash = hash->parent;
			continue;
		}

		while(aux->prox){

			/* Already Exists */
			if(symTNameEquals(aux->lexeme, lookup) == 1){
				return aux;
			}

			/* Keep looking */
			aux = aux->prox;
		}

		/* Already Exists */
		if(symTNameEquals(aux->lexeme, lookup) == 1){
			return aux;
		}
		last_table = hash;
		hash = hash->parent;
	}

	return NULL;
}

void updateDef(symEntry* entry, unsigned short line){
	int len = symTGetNDef(entry);
	entry->def = (unsigned short*)realloc(entry->def, (len+1) * sizeof(unsigned short));
	entry->def[len] = line;
	entry->ndef = len+1;
	return;
}

void symTUpdateCall(symEntry* entry, unsigned short line){
	int len = symTGetNCall(entry);
	entry->call = (unsigned short*)realloc(entry->call, (len+1) * sizeof(unsigned short));
	entry->call[len] = line;
	entry->ncall = len+1;
	return;
}

unsigned short symTGetNCall(symEntry* entry){
	return entry->ncall; 
}
unsigned short symTGetNDef(symEntry* entry){
	return entry->ndef;
}

symTable* symTNewEnv(symTable* hash, char* scope){
	symTable *no;
	int pos;

	if(hash == NULL)
		return NULL;


	hash->child = (symTable*)realloc(hash->child, (hash->len + 1) * sizeof(symTable));
	pos = hash->len++;
	no = hash->child;

	no[pos].table = (symEntry*)malloc(NSYMHASH * sizeof(symEntry));
	no[pos].child = NULL;
	no[pos].parent = hash;
	no[pos].len = 0;
	no[pos].scope = strdup(scope);

	for(int i = 0; i < NSYMHASH;i++)
		assignDefault(&no[pos].table[i]);

	return &no[pos];
}

void symTSetParam(symEntry* entry, int n, ...){
	va_list ptr;
	va_start(ptr, n);

	// Alloc	
	entry->param = (Token*)realloc(entry->param, n * sizeof(Token));
	
	// Assign
	for(int i = 0; i < n; i++)
		entry->param[i] = va_arg(ptr, Token);
	
	va_end(ptr);
	
	entry->nparam = n;
	return;
}

void symTAddParam(symEntry* entry, Token param){
	int n;
	n = symTGetNParam(entry);
	// Alloc	
	entry->param = (Token*)realloc(entry->param, (n+1) * sizeof(Token));
	
	entry->param[n] = param;
	entry->nparam++;
}

int symTGetNParam(symEntry* entry){
	return entry->nparam; 
}

void printSingleDetail(symEntry* entry){
	if(!entry)
		return;

	 char* lexeme = (entry->lexeme)? entry->lexeme : "###";
	
	 // Id - lexeme - type - attr
	 printf("[%s][%s][%s][%d]", tokenStr(entry->id), lexeme, tokenStr(entry->type), entry->attr);

	 // Params
	 printf("[");
	 for(int i = 0; i < symTGetNParam(entry); i++)
		 printf("%s, ", tokenStr(entry->param[i]));
	 printf("]");

	 // Call
	 printf("[");
	 for(int i = 0; i < symTGetNCall(entry); i++)
		 printf("%d, ", entry->call[i]);
	 printf("]");
	 
	 // Def
	 printf("[");
	 for(int i = 0; i < symTGetNDef(entry); i++)
		 printf("%d, ", entry->def[i]);
	 printf("]");

	 // ptr_env - null - ptr_prox
	 printf("[%p][%d][%p]", entry->env, entry->null, entry->prox);
	 printf("\n");

}



void symTPrint(symTable* hash, int deep){
	symEntry* aux;
	
	if(!hash)
		return;
	
	// Print as a tree
	for(int j = 0; j < 4*deep; j++)
		printf(" ");

	aux = hash->table;

	printf("SCOPE - %s\n", hash->scope);
	printf("[ID][Lexeme][Type][Attr][Params][Call][Def][ptr_env][null][ptr_prox]\n");

	// Runs over entry table and print
	for(int i = 0; i < NSYMHASH; i++){
		aux = &hash->table[i];
		
		while(aux){
			if(aux->null){
				aux = aux->prox;
				continue;
			}

			// Print as a tree
			for(int j = 0; j < 4*deep; j++)
				printf(" ");

			// Print item
			printSingleDetail(aux);
			aux = aux->prox;
		}
	}

	// Calls deep env
	for(int i = 0; i < hash->len; i++)
		symTPrint(&hash->child[i], deep+1);

	return;

}


int symTSave(symTable* hash, char* path){
	FILE* fd;

	if(!path || !hash)
		return 0;

	if(!(fd = fopen(path, "w")))
		return -1;

	saveInfo(hash, 0, fd);


	fclose(fd);
	return 1;
}

void saveSingleInfo(symEntry* entry, FILE* fd){
	if(!entry || !fd)
		return;

	 char* lexeme = (entry->lexeme)? entry->lexeme : "###";
	
	 // Id - lexeme - type - attr
	 fprintf(fd, "[%s] [%s] [%s] [%d]", tokenStr(entry->id), lexeme, tokenStr(entry->type), entry->attr);

	 // Params
	fprintf(fd, "[");
	for(int i = 0; i < symTGetNParam(entry); i++)
		fprintf(fd, "%s, ", tokenStr(entry->param[i]));
	fprintf(fd, "]");

	 // Call
	fprintf(fd, "[");
	for(int i = 0; i < symTGetNCall(entry); i++)
		fprintf(fd, "%d, ", entry->call[i]);
	fprintf(fd, "]");
	 
	 // Def
	fprintf(fd, "[");
	for(int i = 0; i < symTGetNDef(entry); i++)
		fprintf(fd, "%d, ", entry->def[i]);
	fprintf(fd, "]");
	fprintf(fd, "\n");

}

void saveInfo(symTable* hash, int deep, FILE *fd){
	symEntry* aux;
	
	if(!hash || !hash->table)
		return;
		
	fprintf(fd, "----------------------[%d - DEEP]--------------------------------\n", deep);
	fprintf(fd, "----------------------[SCOPE - %s]--------------------------------\n", hash->scope);
	
	for(int i = 0; i < 4*deep; i++)
		fprintf(fd, " ");

	fprintf(fd, "[TOKEN] [LEXEME] [TYPE] [ATTR] [PARAMS] [call] [def]\n");
	
	// Runs over entry table and print
	for(int i = 0; i < NSYMHASH; i++){
		aux = &hash->table[i];
		
		while(aux){
			if(aux->null){
				aux = aux->prox;
				continue;
			}

			// Token always exists, lexeme always exists,type always exist
			for(int i = 0; i < 4*deep; i++)
				fprintf(fd, " ");

			saveSingleInfo(aux, fd);

			aux = aux->prox;
		}
	}
	fprintf(fd, "------------------------------------------------------\n");

	// Calls deep env
	for(int i = 0; i < hash->len; i++)
		saveInfo(&hash->child[i], deep+1, fd);

	return;
}

symTable* symTExit(symTable* hash){
	if(!hash)
		return NULL;

	return hash->parent;
}


/*
int main(){
	symEntry* entry;
	symTable* table, *env;
	table = symTInit();
	env = table;
	entry = symTPut(
			table,
			FUN_K,
			strdup("main"),
			INT_K,
			0,
		       	0,
			_DECLARATION
			);

	env = symTNewEnv(table, strdup("main"));
	entry = symTPut(
			env,
			VAR_K,
			strdup("a"),
			INT_K,
			0,
		       	0,
			_DECLARATION
			);
	env = symTExit(env);
	symTPrint(table, 0);


	printf("EOF\n");
}
*/
