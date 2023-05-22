#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"

// TOKEN
#include "../ast/ast.h"

symTable* symTInit(){
	symTable* env;

	env = (symTable*) malloc(sizeof(symTable));

	env->table = (symEntry*)malloc(NSYMHASH * sizeof(symEntry));
	env->child = NULL;
	env->parent = env;

	for(int i = 0; i < NSYMHASH; i++){
		env->table[i].tok = BLANK;
		env->table[i].lexeme = NULL;
		env->table[i].type = BLANK;
		env->table[i].null = 1;
		env->table[i].prox = NULL;
	}

	env->len = 0;
	env->scope = strdup("GLOBAL");
	return env;
}

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
}

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

symEntry symTNewNo(Token tok, char* lexeme, Token type){
	symEntry no;

	no.tok = tok;
	no.lexeme = strdup(lexeme);
	no.type = type;
	no.null = 0;
	no.prox = NULL;

	return no;
}

int symTIsEqual(symEntry *it1, symEntry *it2){

	if((it1->lexeme && !it2->lexeme) || (!it1->lexeme && it2->lexeme))
		return -1;

	if(it1->lexeme && it2->lexeme)
		if(strcmp(it1->lexeme, it2->lexeme) != 0)
			return -1;
		
	return 1;
}

int symTPut(symTable* hash, Token tok, char* lexeme, Token type){
	symEntry toPut, *aux;
	int key = -1;

	if(!hash || !hash->table)
		return -1;

	toPut = symTNewNo(tok, lexeme, type);
	key = symTKey(&toPut);

	aux = &hash->table[key];

	if(aux->null == 1){
		*aux = toPut;
		return 1;
	}

	while(aux->prox){

		/* Already Exists */
		if(symTIsEqual(aux, &toPut) == 1)
			return 0;	

		/* Keep looking */
		aux = aux->prox;
	}

	/* Already Exists */
	if(symTIsEqual(aux, &toPut) == 1)
		return 0;

	aux->prox = (symEntry*)malloc(sizeof(symEntry));
	*(aux->prox) = toPut;
	return 1;
}

symEntry* symTLook(symTable* hash, Token tok, char* lexeme, Token type){
	symTable* auxTable;
	symEntry toLook, *aux;
	int key;
	toLook = symTNewNo(tok, lexeme, type);
	if(!hash || !hash->table)
		return NULL;

	key = symTKey(&toLook);

	auxTable = hash;
	aux = &auxTable->table[key];

	// Check constant over parent table 
	while(auxTable->parent != auxTable){
		while(aux){
			if(symTIsEqual(aux, &toLook) == 1)
				return aux;

			aux = aux->prox;
		}
		
		auxTable = auxTable->parent;
		aux = &auxTable->table[key];

	}

	// Check last table
	while(aux){
		if(symTIsEqual(aux, &toLook) == 1)
			return aux;

		aux = aux->prox;	
	}
	return NULL;
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

	for(int i = 0; i < NSYMHASH;i++){
		no[pos].table[i].tok = BLANK;
		no[pos].table[i].lexeme = NULL;
		no[pos].table[i].type = BLANK;
		no[pos].table[i].null = 1;
		no[pos].table[i].prox = NULL;
	}
	return &no[pos];
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
			printf("[Token, lexeme, type, null, prox, ptr] - [%s, %s, %s, %d, %p, %p]\n", tokenStr(aux->tok), aux->lexeme, tokenStr(aux->type), aux->null, aux->prox, aux);
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

	symTDeepSave(hash, 0, fd);


	fclose(fd);
	return 1;
}

void symTDeepSave(symTable* hash, int deep, FILE *fd){
	symEntry* aux;
	
	if(!hash || !hash->table)
		return;
		
	fprintf(fd, "----------------------[%d - DEEP]--------------------------------\n", deep);
	fprintf(fd, "----------------------[SCOPE - %s]--------------------------------\n", hash->scope);
	
	for(int i = 0; i < 4*deep; i++)
		fprintf(fd, " ");

	fprintf(fd, "[TOKEN] [LEXEME] [TYPE]\n");
	
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

			fprintf(fd, "[%s]\t [%s]\t\t [%s]\n", tokenStr(aux->tok), aux->lexeme, tokenStr(aux->type));

			aux = aux->prox;
		}
	}
	fprintf(fd, "------------------------------------------------------\n");

	// Calls deep env
	for(int i = 0; i < hash->len; i++)
		symTDeepSave(&hash->child[i], deep+1, fd);

	return;
}

symTable* symTExit(symTable* hash){
	if(!hash)
		return NULL;

	return hash->parent;
}

/*
int main(){
	symTable* table, *track;
	table = symTInit();
	symTPut(table, VAR_K, "minhavar", "INT");
	symTPut(table, VAR_K, "minhavar", "VOID");
	track = symTNewEnv(table, "fun1");
	symTPut(track, VAR_K, "outravar", "INT");
	symTPrint(table, 0);
	symTFree(table);
	printf("EOF\n");
}
*/
