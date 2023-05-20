#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strtable.h"

strTable* strTInit(){
	strTable* hash; 
	
	hash = (strTable*)malloc(sizeof(strTable));
	hash->table = (strEntry*) malloc(sizeof(strEntry)*NSTRHASH);
	hash->len = 0;

	// Setting null all prox
	for(int i = 0; i < NSTRHASH; i++){
		hash->table[i].prox = NULL;
		hash->table[i].null = 1;
	}

	return hash;
}

int strTFLL(strEntry* no){
	strEntry *aux, *auxprev;

	// not NULL
	if(no){
		free(no->word);
		return 1;
	}


	aux = no->prox;

	while(aux != NULL){
		auxprev = aux;
		aux = aux->prox;
		free(auxprev->word);
		free(auxprev);
	}

	return 1;
}

int strTFree(strTable* hash){
	strEntry *head;

	if(!hash)
		return 1;

	
	head = hash->table;

	if(!head){
		free(hash);
		hash = NULL;
		return 1;
	}


	// Entries not null
	for(int i = 0; i < NSTRHASH; i++){
			if(!strTFLL(&head[i])){
			printf("ERROR: strTFLL\n");
			return -1;
		}
	}

	free(head);
	free(hash);
	hash = NULL;

	return 1;
}

int strTKey(char* word){
	if(word == NULL || strlen(word) == 0){
		printf("ERROR: Hash key impossible to calculate\n");
		return -1;
	}

	int hsum = 0;

	// Letter's code sum
	for(int i = 0; i < strlen(word); i++){
		hsum += (int)word[i];
	}

	hsum = hsum % NSTRHASH;
	return hsum;
}

strEntry* strTPut(strTable* hash, char* word){
	strEntry* aux;
	int key;

	// if hash or table is null
	if(!hash || !hash->table)
		return NULL;

	key = strTKey(word);

	/*key error*/
	if(key == -1){
		printf("ERROR: strTKey()\n");
		return NULL;
	}

	/* Empty item allocated */
	if(hash->table[key].null == 1){
		hash->table[key].word = strdup(word);
		hash->table[key].null = 0;
		return &hash->table[key];
	}
	/* Word matched with first item of list */
	else if(strcmp(hash->table[key].word, word) == 0){
		return &hash->table[key];
	}

	aux = &hash->table[key];

	/* Look null or word to match */
	while(aux->prox){
		aux = aux->prox;

		/* Word matched */
		if(strcmp(aux->word, word) == 0)
			return aux;
	}

	aux->prox = strTNewNo(word);
	hash->len++;

	return aux->prox;
}

strEntry* strTNewNo(char* word){
	strEntry* no;

	no = (strEntry*)malloc(sizeof(strEntry));
	no->prox = NULL;
	no->null = 0;
	no->word = strdup(word);

	return no;

}

int strTPrint(strTable* hash){
	int count = 0;
	strEntry* list;

	printf("**STRHASH BEGIN**\n");
	for(int i = 0; i < NSTRHASH; i++){
		list = &hash->table[i];

		if(list->null == 0){
			printf("[%d, %p] %s\n", count++, list, list->word);
		}
		list = list->prox;

		while(list){
			printf("[%d, %p] %s\n", count++, list, list->word);
			list = list->prox;
		}
	}
	printf("**STRHASH END**\n");
	printf("\n");
	return 1;
}

/*
int main(){
	strTable* root;
	root = strTInit();

	strTPut(root, "Ola");
	strTPut(root, "Ola00");
	strTPut(root, "minhavariavel");
	strTPut(root, "Eae meu xoven");
	strTPut(root, "Esse é um comentario interessante");
	strTPut(root, "minhavariavel");
	strTPut(root, "strTable");
	
	strTPrint(root);
	strTFree(root);

	printf("EOF\n");
}
*/
