#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#define MALLOC_VALIDATE "Error, malloc allocation did not work properly", __func__
#define REALLOC_VALIDATE "Error, realloc allocation did not work properly", __func__

typedef struct{
	char** list;
	int len;
}listString;

typedef struct Stack{
	int id;
	struct Stack* next;
}stack;

char* lexformat(char* tok, char* lex);

/* Check if ptr was indeed allocated */
void allocateValidator(void** ptr, const char* msg, const char* func);

/* Free pointer and set null */
void freeNull(void** ptr);

listString* newListString();
int addListString(listString* ls, char* str);
int getKeyListString(listString* ls, char* str);
void freeListString(listString* ls);

stack* addStack(stack* ptr);
int popStack();

#endif
