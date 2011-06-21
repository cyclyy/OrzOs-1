#ifndef EXCEPT_H
#define EXCEPT_H

#include "sysdef.h"
#include "interrupt.h"

struct ExceptTable
{
    u64int errorRip, fixupRip;
};

extern u64int copyUnsafe(void *dst,void *src,u64int size);

extern struct ExceptTable exceptTable, exceptTableEnd;

void commonFaultHandler(struct RegisterState *regs);

void pageFaultHandler(struct RegisterState *regs);

void initCpuExceptions();

#endif /* EXCEPT_H */
