#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variadic
#include <stdarg.h>

#include "symtab.h"

// TOKEN
#include "../ast/ast.h"

/*Add new ref and return pointer */
int* addRef(int* nref, int* ref, int line);

void assignState(
		symEntry* entry, 
		Token id, 
		char* lexeme, 
		Token type, 
		unsigned short attr, 
		Token* param, 
		unsigned short* call, 
		unsigned short* def, 
		symTable* env, 
		int null, 
		unsigned short sd_idx, 
		unsigned short sc_idx, 
		symEntry* prox
		);

symTable* symTInit();

void assignEntry(symEntry* entry, Token id, char* lexeme, Token type, unsigned short attr);

void assignDefault(symEntry* entry);
symEntry* put(symTable* hash, Token tok, char* lexeme, Token type, int attr, int line);

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
		symTable* env, 
		int null, 
		unsigned short sd_idx, 
		unsigned short sc_idx, 
		symEntry* prox
		){
         entry->id = id;
         entry->lexeme = (lexeme) ? strdup(lexeme) : NULL; 
         entry->type = type;
	 entry->attr = attr;
	 entry->param = param;
         entry->call = call;
         entry->def = def;
         entry->env = env;
         entry->null = null; 
         entry->sd_idx = sd_idx;
         entry->sc_idx = sc_idx;
         entry->prox = prox; 
	return;
}

void assignEntry(symEntry* entry, Token id, char* lexeme, Token type, unsigned short attr){
         entry->id = id;
         entry->lexeme = (lexeme) ? strdup(lexeme) : NULL; 
         entry->type = type;
         entry->attr = attr;
	 entry->null = 0;
	 return;
}

void assignDefault(symEntry* entry){
	assignState(entry, BLANK, NULL, BLANK, 0, NULL, NULL, NULL, NULL, 1, 0, 0,NULL);
	return;
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
/*
symEntry symTNewNo(Token tok, char* lexeme, Token type, int attr){
	symEntry no;

	no.tok = tok;
	no.lexeme = strdup(lexeme);
	no.type = type;
	no.attr = attr;
	no.null = 0;
	no.prox = NULL;
	no.nref = 0;
	no.ref = NULL;
	no.duplicata = 0;

	return no;
}
*/
int symTNameEquals(char* n1, char* n2){
	if((n1 && !n2) || (!n1 && n2))
		return -1;

	// If name is different
	if((n1 && n2) && (strcmp(n1, n2) != 0))
		return -1;

	return 1;

}
/*
int symTIsEqual(symEntry *it1, symEntry *it2){

	if((it1->lexeme && !it2->lexeme) || (!it1->lexeme && it2->lexeme))
		return -1;

	if(it1->lexeme && it2->lexeme)
		if(strcmp(it1->lexeme, it2->lexeme) != 0)
			return -1;
		
	return 1;
}*/

symEntry* putLocal(symTable* hash, Token tok, char* lexeme, Token type, int attr, int line){
	symEntry toPut, *aux;
	int key = -1;

	if(!hash || !hash->table)
		return NULL;

	assignDefault(&toPut);				// Default state
	assignEntry(&toPut, tok, lexeme, type, attr);	// Add main info
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

symEntry* putGlobal(symTable* hash, Token tok, char* lexeme, Token type, int attr, int line){
	symEntry* entry;
	if(!hash)
		return;
	
	while(hash->table != hash->parent){
		entry = putLocal(hash->table, tok, lexeme, type, attr, line);

		if(entry)
			return entry;

	}
	return NULL;
}

void symTPut(symTable* hash, Token tok, char* lexeme, Token type, int attr, int line, int VAR_CATEGORY){
	symEntry* entry;

	entry = put(
			symTable* hash,
			Token tok,
			char* lexeme,
			Token type,
			int attr,
			int line,
			int VAR_CATEGORY
			);
	if(!entry) return;

	switch(VAR_CATEGORY){
		case V_DEFINITION:
			entry = putLocal(
					hash,
					tok,
					lexeme,
					type,
					attr,
					line,
					VAR_CATEGORY
					);
			updateDef(entry, line);
			break;
		case V_CALL:
			entry = putGlobal(
					hash,
					tok,
					lexeme,
					type,
					attr,
					line,
					VAR_CATEGORY
					);
			updateCall(entry, line);
			break;
		default:
			continue;
	}
	return;
}
void updateDef(symEntry* entry, int line){
	int len = symTGetNDef(entry);
	entry->def = (unsigned short*)realloc(entry->def, (len+1) * sizeof(unsigned short));
	entry->def[len] = line;
	return;
}
void updateCall(symEntry* entry, int line){
	int len = symTGetNCall(entry);
	entry->def = (unsigned short*)realloc(entry->def, symTGetNCall(entry) * sizeof(unsigned short));
	entry->call[len] = line;
	return;
}

unsigned short symTGetNCall(symEntry* entry){
	return sizeof(entry->call)/sizeof(unsigned short);
}
unsigned short symTGetNDef(symEntry* entry){
	return sizeof(entry->def)/sizeof(unsigned short);
}
/*
symEntry* symTLook(symTable* hash, Token tok, char* lexeme, Token type){
	symTable* auxTable;
	symEntry toLook, *aux;
	int key;
	toLook = symTNewNo(tok, lexeme, type, 0);
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
*/
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

	return;
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
			printf(
			"[Token, lexeme, type, attr, null, prox, ptr, duplicata, nref, [ref]]"
		       	"- [%s, %s, %s, %d, %d, %p, %p, %d, %d,[",
					tokenStr(aux->tok),
				      	aux->lexeme,
				      	tokenStr(aux->type),
					aux->attr,
				      	aux->null,
				      	aux->prox,
					aux,
					aux->duplicata,
					aux->nref
			);
			for(int i = 0; i < aux->nref; i++){
				printf("%d, ", aux->ref[i]);
			}
		       	printf("]\n");
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

	fprintf(fd, "[TOKEN] [LEXEME] [TYPE] [ATTR] [nref] [ref]\n");
	
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

			fprintf(fd, "[%s]\t [%s]\t\t [%s]\t [%d]\t [%d]\t[",
					tokenStr(aux->tok),
					aux->lexeme,
					tokenStr(aux->type),
					aux->attr,
					aux->nref
			);
			for(int i = 0; i < aux->nref; i++){
				fprintf(fd, "%d, ", aux->ref[i]);
			} 
			fprintf(fd, "]\n");

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


/*	PRIVATE		*/
int* addRef(int* nref, int* ref, int line){
	ref = (int*)realloc(ref, (*nref+1) * sizeof(int));
	ref[(*nref)++] = line;
	return ref;
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
