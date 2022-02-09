#include <stdio.h>
#include <stdlib.h>
#include "file_log.h"

FILE *scan_log, *p_log, *sem_log;

FILE* open_log(int wlog){
	FILE* fp;
	char* fullpath;

	if(wlog == LOG_SCANNER)
		fullpath = PATH_OUTPUT SCANNER_FNAME;

	else if(wlog == LOG_PARSER)
		fullpath = PATH_OUTPUT PARSER_FNAME;
	
	else if(wlog == LOG_SEMANTIC)
		fullpath = PATH_OUTPUT SEMANTIC_FNAME;
	
	else{
		printf("ERROR Log type unknow\n");
	}

	fp = fopen(fullpath, "w");

	if(fp == NULL)
		printf("ERROR oppening log\n");

	return fp;
}
