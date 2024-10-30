#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "nameManager.h"

#define BATCH_LEN_NAMES 50

extern ArrayString nameManager;
static void resize();
static void freeNames();

void nmInit(){
	nameManager.len = 0;
	nameManager.names = calloc(BATCH_LEN_NAMES, sizeof(char*));
	if(nameManager.names == NULL){
		printf("ERROR - Allocating mem to Name Manager didn't succeded\n");
		exit(0);
	}
}

char* nmAdd(char *str){
	char* hold;
	hold = strdup(str);

	if(!nameManager.names) return NULL;

	for(int i = 0; i < nameManager.len; i++){
		if(strcmp(str, nameManager.names[i]) == 0) return nameManager.names[i];
	}

	// Dinamically expand size to allow more names
	if(((nameManager.len+1) % BATCH_LEN_NAMES) == 0) resize();

	if(!hold){
		printf("ERROR - Allocating new string to Name Manager failed\n");
		exit(0);
	}

	// Store pointer
	nameManager.names[nameManager.len++] = hold;
	return str;
}


/**
 * Resize array string based on batch size and array len
*/
void nmClean(){
	if(nameManager.names) freeNames();
	nameManager.len = 0;
}

/**
 * Free memory allocate to name field **WHITOUT** changing len property
 */
static void resize(){
	static int batches = 2;
	char** hold;
	hold = (char**) realloc(nameManager.names, batches++ * BATCH_LEN_NAMES * sizeof(char*));
	if(hold == NULL){
		printf("Name Manager resize went wrong, aborting...\n");
		exit(0);
	}
	nameManager.names = hold;
}

static void freeNames() {
	if(!nameManager.names){
		return;
	}
	
	// Free every name
	for(int i = 0; i < nameManager.len; i++){
		free(nameManager.names[i]);
		nameManager.names[i] = NULL;
	}

	// Free name structure
	free(nameManager.names);
	nameManager.names = NULL;
}

void nmShow(){
	for(int i = 0; i < nameManager.len; i++){
		printf("%s\n", nameManager.names[i]);
	}
}

// int main(){
// 	nmInit();
// 	char* str;
// 	int in;
// 	int garbage;
// 	bool loop = true;
	
// 	while(loop){
// 		printf("Digite:\n");
// 		printf("1. Adicionar nome na lista \n");
// 		printf("2. Visualizar lista \n");
// 		printf("3. Limpar lista \n");
// 		printf("4. Sair \n");
// 		scanf("%d", &in);
// 		system("clear");

// 		switch(in){
// 			case 1:
// 				printf("Digite uma string para adicionar na estrutura\n");
// 				scanf("%s", str);
// 				nmAdd(str);
// 			break;

// 			case 2:
// 				show();
// 				getchar();
// 				getchar();
// 			break;

// 			case 3:
// 				nmClean();
// 			break;

// 			case 4:
// 				loop = false;
// 			break;
// 			default:
// 				printf("Option not available\n");
// 		}
// 	}
// }