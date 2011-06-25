#ifndef PROGRAM_H
#define PROGRAM_H

#include "sysdef.h"

struct Program {
    s64int ref;
    u64int entry;
    u64int brk;
    u64int allocBefore;
};

struct Program *progCreate();

struct Program *progRef();

s64int progDeref();

#endif /* PROGRAM_H */
