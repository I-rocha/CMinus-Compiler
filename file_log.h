#ifndef FILE_LOG_H
#define FILE_LOG_H

#define PATH_OUTPUT "output/"

#define SCANNER_FNAME "scanner_log.txt"
#define PARSER_FNAME "parser_log.txt"
#define SEMANTIC_FNAME "semantic_log.txt"

#define LOG_SCANNER 0
#define LOG_PARSER 1
#define LOG_SEMANTIC 2

FILE *scan_log, *p_log, *sem_log;

FILE* open_log(int wlog);
#endif
