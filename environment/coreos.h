#ifndef COREOS_H
#define COREOS_H

#include "../cgen/cgen.h"
#include "../utils.h"
#include "mnemonic.h"

int osCall(quad* fun, memmory* ram, stack* params);

#endif