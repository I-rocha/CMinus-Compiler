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
