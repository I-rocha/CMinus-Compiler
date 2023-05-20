#ifndef STRTABLE_H
#define STRTABLE_H

#define NSTRHASH 1000

typedef struct strEntry{
	char* word;
	struct strEntry* prox;
	int null;
} strEntry;

typedef struct {
	int len;
	strEntry* table;
} strTable;

/*
 * */
strTable* strTInit();

/* 
 * String Table Free Linked List
 * */
int strTFLL(strEntry* no);

/*
 * */
int strTFree(strTable* hash);

/*
 * */
int strTKey(char* word);

/*
 * */
strEntry* strTNewNo(char* word);

/*
 * ptr: If inserted or word match
 * NULL: If fails
 * */
strEntry* strTPut(strTable* hash, char* word);

/*
 * */
int strTPrint(strTable* hash);

#endif
