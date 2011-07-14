#ifndef UTIL_H
#define UTIL_H

#include "sysdef.h"

#define MAGIC_BREAK() asm volatile("xchg %bx,%bx")
#define DBG(...) do { printk("%s:%d:",__FUNCTION__,__LINE__); printk(__VA_ARGS__);printk("\n"); } while (0)
#define PANIC(...) do { DBG(__VA_ARGS__); for(;;); } while(0)

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

void memset(void *addr, u8int c, u64int n);

void *memcpy(void *dst, void *src, u64int n);

char *strcpy(char *dst, const char *src);

s64int strcmp(const char *s1, const char *s2);

s64int strncmp(const char *s1, const char *s2, u64int n);

u64int strlen(const char *s);

// the first apperance of s2 in s1
char *strstr(const char *s1, const char *s2);

// the last apperance of s2 in s1
char *strrstr(const char *s1, const char *s2);

// the first apperance of c in s1
char *strchr(const char *s1, char c);

// the last apperance of c in s1
char *strrchr(const char *s1, char c);

char *strdup(const char *s);

void printk(const char *fmt, ...);

u64int sprintf(char *buf, char *fmt, ...);

char *dirname(char *dest, const char *s);
char *basename(char *dest, const char *s);

u64int strtoul(const char *cp,char **endp,u32int base);

s64int strtol(const char *cp,char **endp,u32int base);

s32int atoi(const char *nptr);

#define atol(x) atoi(x)

char *ltoa(s64int i, char *buf, int base);

char *ultoa(u64int i, char *buf, int base);

void clearBit(u64int *x, u64int i);

void setBit(u64int *x, u64int i);

#define abs __builtin_abs

void insl(u16int port, void *buffer, u32int quads);

void insw(u16int port, void *buffer, u32int words);

void insb(u16int port, void *buffer, u32int bytes);

void outsl(u16int port, void *buffer, u32int quads);

void outsw(u16int port, void *buffer, u32int words);

void outsb(u16int port, void *buffer, u32int bytes);

#endif /* UTIL_H */
