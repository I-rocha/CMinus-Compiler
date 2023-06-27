#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define LPADDING 10

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

listString newListString(){
	listString ls;

	ls.list = NULL;
	ls.len = 0;
	return ls;
}

int addListString(listString ls, char* str){
	ls.list = (char**)realloc(ls.list, sizeof(char*) * (ls.len+1));
	
	if(!ls.list){
		printf("Error, allocating didn't work (addListString)\n");
		exit(0);
		return -1;
	}

	ls.list[ls.len++] = strdup(str);
	return 1;
}

int getKeyListString(listString ls, char* str){
	for(int i = 0; i < ls.len; i++){
		if(strcmp(ls.list[i], str) == 0)
			return i;
	}
}

void freeListString(listString ls){
	if(!ls.list)
		return;

	for(int i = 0; i < ls.len; i++){
		if(!ls.list[i])
			continue;
		free(ls.list[i]);
	}
	free(ls.list);
	ls.list = NULL;
	ls.len = 0;
}
