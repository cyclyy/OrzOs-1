#ifndef DEBUGCON_H
#define DEBUGCON_H

#include "sysdef.h"

// put a char
void dbgPutch(char c);

// put a string
void dbgPuts(const char *s);

// put a line of  s: 0xp
//void dbgputp(const char *s, void *p);

// put a dec num
void dbgPutn(u64int i);

// put a hex num
void dbgPuthex(u64int i);

// printf simple clone
//void printk(char *fmt, ...);

#endif

