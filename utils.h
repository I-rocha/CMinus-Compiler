#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#define MALLOC_VALIDATE "Error, malloc allocation did not work properly", __func__
#define REALLOC_VALIDATE "Error, realloc allocation did not work properly", __func__

char* lexformat(char* tok, char* lex);


void allocateValidator(void** ptr, const char* msg, const char* func);
void freeNull(void** ptr);

#endif
