#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#define MALLOC_VALIDATE "Error, malloc allocation did not work properly", __func__
#define REALLOC_VALIDATE "Error, realloc allocation did not work properly", __func__

typedef struct Stack{
	char* id;
	struct Stack* next;
}stack;

stack* addStack(stack* ptr, char* id);
char* popStack();

char* lexformat(char* tok, char* lex);

/* Check if ptr was indeed allocated */
void allocateValidator(void** ptr, const char* msg, const char* func);

/* Free pointer and set null */
void freeNull(void** ptr);

int createDir(char* pathName);


#endif
