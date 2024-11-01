#ifndef NAMEMANAGER_H
#define NAMEMANAGER_H

typedef struct {
	char** names;
	int len;
}ArrayString;

void nmInit();

/**
 * Allocate a new string to the structure
 */
char* nmAdd(char *str);

/**
 * clear struct
 */
void nmClean();

void nmShow();
#endif
