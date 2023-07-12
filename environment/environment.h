#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../cgen/cgen.h"

void envInitGlobal();
void endEnv();

void toAssembly(quad* head);
void saveAssembly(const char* path);
void saveAssemblyPretty(const char* path);

#endif