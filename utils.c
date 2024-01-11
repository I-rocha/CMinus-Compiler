#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Directory
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#define LPADDING 10

struct stat st = {0};

char* lexformat(char* tok, char* lex){
	int sz;
	char hollow[] = " ";

	sz = strlen(tok);
	
	while(sz < LPADDING){
		strcat(tok, hollow);
		sz++;
	}
	strcat(tok, lex);
	
	return strcat(tok, "\n");
}

void freeNull(void** ptr){
	if(!*ptr)
		return;

	free(*ptr);
	*ptr = NULL;
	
	return;
}

void allocateValidator(void** ptr, const char* msg, const char* func){
	char err_txt[100];
	sprintf(err_txt, "%s. (%s)\n", msg, func);

	if(*ptr)
		return;
	printf("%s", err_txt);
	exit(0);
}

stack* addStack(stack* ptr, char* id){
	stack* new;

	// Allocate new struct
	new = (stack*)malloc(sizeof(stack));
	allocateValidator((void**)&new, MALLOC_VALIDATE);

	new->id = id;

	// Check if ptr already exist and update next
	if(!ptr){
		new->next = NULL;
	}
	else
		new->next = ptr;
	
	ptr = new;

	return ptr;
}

char* popStack(stack** ptr){
	stack* toRemove;
	char* ret;

	if(!ptr || !*ptr)
		return NULL;
	
	ret = (*ptr)->id;

	toRemove = *ptr;
	*ptr = (*ptr)->next;

	freeNull((void**)&toRemove);
	return ret;
}


int createDir(char* pathName){
	if(stat(pathName, &st) != -1)
		return 1;

	if(mkdir(pathName, 0700) == -1){
		printf("Error creating dir (%s)\n", pathName);
		return -1;
	}

	return 1;
}

int isReg(char* str){
	return (str[0] == '$')? 1 : 0;
}

int isLabel(char* str){
	return (str[0] == 'L') ? 1 : 0;
}

int getN(char* str){
	if(isReg(str))
		return atoi(&str[2]);

	if(isLabel(str))
		return atoi(&str[1]);

	return atoi(str);
}