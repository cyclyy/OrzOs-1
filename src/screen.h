#ifndef SCREEN_H
#define SCREEN_H

#include "sysdef.h"

// clear screen
void scr_clear();

// put a char
void scr_putch(char c);

// put a string
void scr_puts(const char *s);

// put a line of  s: 0xp
//void scr_putp(const char *s, void *p);

// put a dec num
void scr_putn(u64int i);

// put a hex num
void scr_puthex(u64int i);

// printf simple clone
//void printk(char *fmt, ...);

#endif

