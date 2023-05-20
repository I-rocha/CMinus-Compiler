#include <stdio.h>
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

