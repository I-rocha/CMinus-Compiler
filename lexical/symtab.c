#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"


symTable* symTInit(){
	symTable* env;

	env = (symTable*) malloc(sizeof(symTable));

	env->table = (symEntry*)malloc(NSYMHASH * sizeof(symEntry));
	env->child = NULL;
	env->parent = env;

	for(int i = 0; i < NSYMHASH; i++){
		env->table[i].tok = NULL;
		env->table[i].declaration = NULL;
		env->table[i].attrID = NULL;
		env->table[i].attrINT = NULL;
		env->table[i].type = NULL;
		env->table[i].null = 1;
		env->table[i].prox = NULL;
	}

	env->len = 0;
	return env;
}

int symTFLL(symEntry* no){
	symEntry *aux, *auxprev;

	if(no){
		free(no->tok);
		free(no->declaration);
		free(no->attrID);
		free(no->attrINT);
		free(no->type);	
	}
	aux = no->prox;

	while(aux){
		auxprev = aux;
		aux = aux->prox;
		free(auxprev->tok);
		free(auxprev->attrID);
		free(auxprev->attrINT);
		free(auxprev->type);
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

/*TODO: Implementar melhor*/
int symTKey(symEntry* item){
	int s = 0;

	s += (item->tok == NULL)? 0 : strlen(item->tok);
	s += (item->declaration == NULL)? 0 : strlen(item->declaration);
	s += (item->attrID == NULL)? 0 : strlen(item->attrID);
	s += (item->attrINT == NULL)? 0 : strlen(item->attrINT);
	//s += (item->type == NULL)? 0 : strlen(item->type);
	//s += (item->prox == NULL)? 0 : sizeof(item->prox);
	s = s % NSYMHASH;
	return s;
}

symEntry symTNewNo(char* tok, char* declaration, char* attrID, char* attrINT, char* type){
	symEntry no;

	no.tok = strdup(tok);
	no.declaration = strdup(declaration);
	no.attrID = (attrID) ? strdup(attrID) : NULL;
	no.attrINT = (attrINT) ? strdup(attrINT) : NULL;
	no.type = (type) ? strdup(type) : NULL;
	no.null = 0;
	no.prox = NULL;

	return no;
}

int symTIsEqual(symEntry *it1, symEntry *it2){

	if((it1->tok && !it2->tok) || (!it1->tok && it2->tok))
		return -1;


	if(it1->tok && it2->tok)
		if(strcmp(it1->tok, it2->tok) != 0)
			return -1;
		
	if(it1->attrID && it2->attrID)
		if(strcmp(it1->attrID, it2->attrID) != 0)
			return -1;

	if(it1->attrINT && it2->attrINT)
		if(strcmp(it1->attrINT, it2->attrINT) != 0)
			return -1;
	
	if(it1->declaration && it2->declaration)
		if(strcmp(it1->declaration, it2->declaration) != 0)
			return -1;

	return 1;
}

int symTPut(symTable* hash, char* tok, char* declaration, char* attrID, char* attrINT, char* type){
	symEntry toPut, *aux;
	int key = -1;

	if(!hash || !hash->table)
		return -1;

	toPut = symTNewNo(tok, declaration, attrID, attrINT, type);
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

symEntry* symTLook(symTable* hash, char* tok, char* declaration, char* attrID, char* attrINT){
	symTable* auxTable;
	symEntry toLook, *aux;
	int key;
	toLook = symTNewNo(tok, declaration, attrID, attrINT, NULL);
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

symTable* symTNewEnv(symTable* hash){
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

	for(int i = 0; i < NSYMHASH;i++){
		no[pos].table[i].tok = NULL;
		no[pos].table[i].declaration = NULL;
		no[pos].table[i].attrID = NULL;
		no[pos].table[i].attrINT = NULL;
		no[pos].table[i].type = NULL;
		no[pos].table[i].null = 1;
		no[pos].table[i].prox = NULL;
	}
	return &no[pos];
}

void symTPrint(symTable* hash, int deep){
	char *attr, *type, auxType[10];
	symEntry* aux;
	
	if(!hash)
		return;
	
	aux = hash->table;

	// Runs over entry table and print
	for(int i = 0; i < NSYMHASH; i++){
		aux = &hash->table[i];
		
		while(aux){
			if(aux->null){
				aux = aux->prox;
				continue;
			}
			if(!aux->type){
				strcpy(auxType, "(NIL)");
				type = auxType;
			}
			else
				type = aux->type;

			if(aux->attrID)
				attr = aux->attrID;

			else
				attr = aux->attrINT;
			

			// Print as a tree
			for(int j = 0; j < 4*deep; j++)
				printf(" ");

			// Print item
			printf("[Token, attr, declaration, type, null, prox, ptr] - [%s, %s, %s, %s, %d, %p, %p]\n", aux->tok, attr, aux->declaration, type, aux->null, aux->prox, aux);
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
	char attr[10];
	
	if(!hash || !hash->table)
		return;
		
	fprintf(fd, "----------------------[%d - DEEP]--------------------------------\n", deep);
	
	for(int i = 0; i < 4*deep; i++)
		fprintf(fd, " ");

	fprintf(fd, "[TOKEN] [ATTRIBUTE] [DECLARATION] [TYPE]\n");
	
	// Runs over entry table and print
	for(int i = 0; i < NSYMHASH; i++){
		aux = &hash->table[i];
		
		while(aux){
			if(aux->null){
				aux = aux->prox;
				continue;
			}

			// Token always exists, attribute always exists, declaration always exists, type always exist
			if(aux->attrID)
				strcpy(attr, aux->attrID);
			
			else
				strcpy(attr, aux->attrID);

			for(int i = 0; i < 4*deep; i++)
				fprintf(fd, " ");

			fprintf(fd, "[%s]\t [%s]\t\t [%s]\t\t [%s]\n", aux->tok, attr, aux->declaration, aux->type);

			aux = aux->prox;
		}
	}
	fprintf(fd, "------------------------------------------------------\n");

	// Calls deep env
	for(int i = 0; i < hash->len; i++)
		symTDeepSave(&hash->child[i], deep+1, fd);

	return;
}


/*
int main(){
	symTable* table, *track;
	table = symTInit();
	symTPut(table, "ID", "minhavar", NULL, NULL);
	symTPut(table, "ID", "minhavar", NULL, NULL);
	track = symTNewEnv(table);
	symTPut(track, "NUM", NULL, "37", "int");
	symTPrint(table, 0);
	symTFree(table);
	printf("EOF\n");
}
*/
