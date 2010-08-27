#include "common.h"
#include "screen.h"
#include "kheap.h"

void outb(u16int port, u8int value)
{
    __asm__ __volatile__ ("outb %0, %1" : : "a"(value), "dN"(port) );
}

void outw(u16int port, u16int value)
{
    __asm__ __volatile__ ("outw %0, %1" : : "a"(value), "dN"(port) );
}

void outl(u16int port, u32int value)
{
    __asm__ __volatile__ ("outl %0, %1" : : "a"(value), "dN"(port) );
}

u8int inb(u16int port)
{
    u8int ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "dN"(port) );
    return ret;
}

u16int inw(u16int port)
{
    u16int ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "dN"(port) );
    return ret;
}

u32int inl(u16int port)
{
    u32int ret;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(ret) : "dN"(port) );
    return ret;
}

void insl(u16int port, u32int buffer, u32int quads) {
    u32int i;
    for (i=0; i<quads*4; i+=4) {
        *(u32int*)(buffer+i) = inl(port);
    }
}

void insw(u16int port, u32int buffer, u32int words) {
    u32int i;
    for (i=0; i<words*2; i+=2) {
        *(u16int*)(buffer+i) = inw(port);
    }
}

void insb(u16int port, u32int buffer, u32int bytes) {
    u32int i;
    for (i=0; i<bytes; i++) {
        *(u8int*)(buffer+i) = inb(port);
    }
}

void memset(void *addr, u8int c, u32int n)
{
    while(n) {
        ((u8int *)addr)[--n] = c;
    }
}

void outsl(u16int port, u32int buffer, u32int quads) {
    u32int i;
    for (i=0; i<quads*4; i+=4) {
        outl(port, *(u32int*)(buffer+i));
    }
}

void outsw(u16int port, u32int buffer, u32int words) {
    u32int i;
    for (i=0; i<words*2; i+=2) {
        outw(port, *(u16int*)(buffer+i));
    }
}

void outsb(u16int port, u32int buffer, u32int bytes) {
    u32int i;
    for (i=0; i<bytes; i++) {
        outb(port, *(u8int*)(buffer+i));
    }
}

void *memcpy(void *dst, void *src, u32int n)
{
    while (n) {
        *(u8int*)(dst + n - 1) = *(u8int*)(src + n - 1);
        n--;
    }

    return dst;
}

void panic(const char *msg, const char *file, u32int line)
{
    __asm__ __volatile__("cli");

    scr_puts("\nKernel Panic("); 
    scr_puts(msg);  
    scr_puts(") at "); 
    scr_puts(file); 
    scr_putch(':'); 
    scr_putn(line); 
    while(1);
}

void panic_assert(const char *msg, const char *file, u32int line)
{
    __asm__ __volatile__("cli");

    scr_puts("\nAssert failed("); 
    scr_puts(msg);  
    scr_puts(") at "); 
    scr_puts(file); 
    scr_putch(':'); 
    scr_putn(line); 
    while(1);
}

char *strcpy(char *dst, const char *src)
{
    u32int i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;

    return dst;
}

s32int strcmp(const char *s1, const char *s2)
{
    u32int l1 = strlen(s1);
    u32int l2 = strlen(s2);
    u32int i;

    for  (i=0; i<MIN(l1,l2); i++) {
        if (s1[i] < s2[i])
            return -1;
        else if (s1[i] > s2[i])
            return 1;
    }

    if (l1 < l2)
        return -1;
    else if (l1 > l2)
        return 1;
    else
        return 0;
}

u32int strlen(const char *s)
{
    u32int i = 0;

    while (s[i]) {
        i++;
    }

    return i;
}

char *strstr(char *s1, char *s2)
{
    if (!s1 || !s2)
        return 0;

    u32int n1 = strlen(s1);
    u32int n2 = strlen(s2);

    if (n1 < n2)
        return 0;

    u32int i,j;

    for (i=0; i<=n1-n2; i++) {
        u32int same = 1;
        for (j=0; j<n2; j++) {
            if (s1[i+j] != s2[j]) {
                same = 0;
                break;
            }
        }
        if (same)
            return s1+i;
    }

    return 0;
}

char *strrstr(char *s1, char *s2)
{
    if (!s1 || !s2)
        return 0;

    u32int n1 = strlen(s1);
    u32int n2 = strlen(s2);

    if (n1 < n2)
        return 0;

    u32int i,j;

    for (i=n1-n2; i>=0; i--) {
        u32int same = 1;
        for (j=0; j<n2; j++) {
            if (s1[i+j] != s2[j]) {
                same = 0;
                break;
            }
        }
        if (same)
            return s1+i;
    }

    return 0;
}

// the first apperance of c in s1
char *strchr(char *s1, char c)
{
    if (!s1)
        return 0;

    char *s = s1;
    while (*s && (*s != c)) 
        s++;

    if (*s == c)
        return s;

    return 0;
}

// the last apperance of c in s1
char *strrchr(char *s1, char c)
{
    if (!s1)
        return 0;

    char *s = s1;
    char *ret = 0;

    while (*s) { 
        if (*s == c) {
            ret = s;
        }
        s++;
    }

    return ret;
}

char *strdup(char *s)
{
    char *ret = (char*)kmalloc(strlen(s) + 1);
    strcpy(ret, s);

    return ret;
}

u32int strbrk(char **result, const char *str, const char *delim)
{
    if (!result || !str || !delim)
        return 0;

    u32int n = 0, i, j, slen, sublen;
    char *s = strdup(str);
    slen = strlen(s);
    for (i=0; i<slen; i++) 
        for (j=0; j<strlen(delim); j++)
            if (s[i]==delim[j])
                s[i]=0;
    i=0;
    while (i<slen) {
        if (s[i]) {
            sublen = strlen(s+i);
            result[n++] = strdup(s+i);
            i+=sublen;
        }
        i++;
    }
    result[n] = 0;
    kfree(s);
    return n;
}

char *dirname(char *s)
{
    char *ret = strdup(s);

    u32int n = strlen(ret);

    if (strcmp(ret, "/") == 0) 
        return ret;

    if (ret[n-1] == '/') {
        ret[n-1] = 0;
        n--;
    }

    char *slash = strrchr(ret, '/');

    if (slash == 0) {
        strcpy(ret, ".");
    } else if (slash > ret) {
        *slash = 0;
    } else {
        strcpy(ret, "/");
    }

    return ret;
}

char *basename(char *s)
{
    char *ret = strdup(s);
    u32int n = strlen(ret);

    if (strcmp(ret, "/") == 0) 
        return ret;

    if (ret[n-1] == '/') {
        ret[n-1] = 0;
        n--;
    }

    char *tmp;
    char *slash = strrchr(ret, '/');
    if (slash != 0) {
        tmp = strdup(slash+1);
        kfree(ret);
        ret = tmp;
    }

    return ret;
}

u32int sprintf(char *buf, char *fmt, ...)
{
    if (!fmt || !buf) 
        return 0;

    u32int i,j;
    u32int idx = 0;
    char ch;
    char *s;
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        switch (*fmt) {
            case '%':
                fmt++;
                switch (*fmt) {
                    case '%':
                        buf[idx++] = *fmt;
                        fmt++;
                        break;
                    case 'c':
                        ch = va_arg(ap, char);
                        buf[idx++] = ch;
                        fmt++;
                        break;
                    case 's':
                        s = va_arg(ap, char*);
                        strcpy(buf+idx, s);
                        idx += strlen(s);
                        fmt++;
                        break;
                    case 'd':
                        i = va_arg(ap, u32int);
                        // scr_putn(i);
                        fmt++;
                        break;
                    case 'p':
                    case 'x':
                        //scr_puts("0x");
                        i = va_arg(ap, u32int);
                        //scr_puthex(i);
                        fmt++;
                        break;
                    default:
                        buf[idx++] = '%';
                        buf[idx++] = *fmt;
                        fmt++;
                        break;
                }
                break;
            case '\\':
                fmt++;
                switch (*fmt) {
                    case '\\':
                        buf[idx++] = '\\';
                        fmt++;
                        break;
                    case '"':
                        buf[idx++] = '\"';
                        fmt++;
                        break;
                    case 'b':
                        buf[idx++] = '\b';
                        fmt++;
                        break;
                    case 't':
                        buf[idx++] = '\t';
                        fmt++;
                        break;
                    case 'n':
                        buf[idx++] = '\n';
                        fmt++;
                        break;
                    case 'r':
                        buf[idx++] = '\r';
                        fmt++;
                        break;
                    default:
                        buf[idx++] = '\\';
                        buf[idx++] = *fmt;
                        fmt++;
                        break;
                }
            default:
                buf[idx++] = *fmt++;
        }
    }

    va_end(ap);

    buf[idx] = 0;

    return idx;
}
