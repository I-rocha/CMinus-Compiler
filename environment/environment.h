#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../cgen/cgen.h"

void envInitGlobal();
void endEnv();

void toAssembly(quad* head);

#endif