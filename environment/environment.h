#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../cgen/cgen.h"

void envInitGlobal();
void endEnv();

void toAssembly(quad* head);
void saveBin(const char* path);
void saveAssembly(const char* path);
void saveBinQuartus(const char* path);

#endif