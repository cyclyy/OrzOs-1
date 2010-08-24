#ifndef COMMON_H
#define COMMON_H

#include "errno.h"

#define MAGIC_BREAK() asm volatile("xchg %bx,%bx")

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b))
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b))

#define PANIC(x) panic(x, __FILE__, __LINE__)
#define ASSERT(x) ((x) ? (void)0 : panic_assert(#x, __FILE__, __LINE__))

#define MAX_NAME_LEN  128
#define MAX_PATH_LEN  128

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_list void*
#define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.
typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;

void outb(u16int port, u8int value);

void outw(u16int port, u16int value);

void outl(u16int port, u32int value);

u8int inb(u16int port);

u16int inw(u16int port);

u32int inl(u16int port);

void memset(void *addr, u8int c, u32int n);

void *memcpy(void *dst, void *src, u32int n);

void panic(const char *msg, const char *file, u32int line);

void panic_assert(const char *msg, const char *file, u32int line);

char *strcpy(char *dst, const char *src);

s32int strcmp(const char *s1, const char *s2);

u32int strlen(const char *s);

// the first apperance of s2 in s1
char *strstr(char *s1, char *s2);

// the last apperance of s2 in s1
char *strrstr(char *s1, char *s2);

// the first apperance of c in s1
char *strchr(char *s1, char c);

// the last apperance of c in s1
char *strrchr(char *s1, char c);

char *strdup(char *s);

// support abs path only
char *dirname(char *s);

// support abs path only
char *basename(char *s);

u32int sprintf(char *buf, char *fmt, ...);

#endif
