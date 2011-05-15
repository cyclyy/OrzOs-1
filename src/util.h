#ifndef UTIL_H
#define UTIL_H

#include "sysdef.h"

#define MAGIC_BREAK() asm volatile("xchg %bx,%bx")

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b))
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b))

#define va_list __builtin_va_list
#define va_start(ap,v) __builtin_va_start(ap,v)
#define va_arg(ap,t)   __builtin_va_arg(ap,t)
#define va_end(ap)     __builtin_va_end(ap)

void outb(u16int port, u8int value);

void outw(u16int port, u16int value);

void outl(u16int port, u32int value);

u8int inb(u16int port);

u16int inw(u16int port);

u32int inl(u16int port);

void memset(void *addr, u8int c, u32int n);

char *strcpy(char *dst, const char *src);

u32int strlen(const char *s);

void printk(char *fmt, ...);


/*
void insl(u16int port, u32int buffer, u32int quads);

void insw(u16int port, u32int buffer, u32int words);

void insb(u16int port, u32int buffer, u32int bytes);

void outsl(u16int port, u32int buffer, u32int quads);

void outsw(u16int port, u32int buffer, u32int words);

void outsb(u16int port, u32int buffer, u32int bytes);
*/

#endif
