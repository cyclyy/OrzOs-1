#include "util.h"
#include "screen.h"

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

/*
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

*/
/*
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
*/

void memset(void *addr, u8int c, u64int n)
{
    while(n) {
        ((u8int *)addr)[--n] = c;
    }
}

void *memcpy(void *dst, void *src, u64int n)
{
    while (n) {
        *(u8int*)(dst + n - 1) = *(u8int*)(src + n - 1);
        n--;
    }

    return dst;
}

u64int vsprintf(char *buf, char *fmt, va_list ap)
{
    if (!fmt || !buf) 
        return 0;

    u64int idx = 0;
    int ch, i;
    char *s;

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
                        ch = va_arg(ap, int);
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
                        i = va_arg(ap, u64int);
                        //scr_putn(i);
                        fmt++;
                        break;
                    case 'p':
                    case 'x':
                        //scr_puts("0x");
                        i = va_arg(ap, u64int);
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

    buf[idx] = 0;

    return idx;
}

u64int sprintf(char *buf, char *fmt, ...)
{
    int ret;
    va_list args;
    va_start(args,fmt);
    ret = vsprintf(buf,fmt,args);
    va_end(args);
    return ret;
}

char *strcpy(char *dst, const char *src)
{
    u64int i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;

    return dst;
}

s64int strcmp(const char *s1, const char *s2)
{
    u64int l1 = strlen(s1);
    u64int l2 = strlen(s2);
    u64int i;

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

u64int strlen(const char *s)
{
    u64int i = 0;

    while (s[i]) {
        i++;
    }

    return i;
}

char *strstr(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 0;

    u64int n1 = strlen(s1);
    u64int n2 = strlen(s2);

    if (n1 < n2)
        return 0;

    u64int i,j;

    for (i=0; i<=n1-n2; i++) {
        u64int same = 1;
        for (j=0; j<n2; j++) {
            if (s1[i+j] != s2[j]) {
                same = 0;
                break;
            }
        }
        if (same)
            return (char*)s1+i;
    }

    return 0;
}

char *strrstr(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 0;

    u64int n1 = strlen(s1);
    u64int n2 = strlen(s2);

    if (n1 < n2)
        return 0;

    u64int i,j;

    for (i=n1-n2; i>=0; i--) {
        u64int same = 1;
        for (j=0; j<n2; j++) {
            if (s1[i+j] != s2[j]) {
                same = 0;
                break;
            }
        }
        if (same)
            return (char*)s1+i;
    }

    return 0;
}

// the first apperance of c in s1
char *strchr(const char *s1, char c)
{
    if (!s1)
        return 0;

    while (*s1 && (*s1 != c)) 
        s1++;

    if (*s1 == c)
        return (char*)s1;

    return 0;
}

// the last apperance of c in s1
char *strrchr(const char *s1, char c)
{
    if (!s1)
        return 0;

    char *ret = 0;

    while (*s1) { 
        if (*s1 == c) {
            ret = (char*)s1;
        }
        s1++;
    }

    return ret;
}

/*
char *strdup(const char *s)
{
    char *ret = (char*)kMalloc(strlen(s) + 1);
    strcpy(ret, s);

    return ret;
}
*/

void printk(char *fmt, ...)
{
    if (!fmt) 
        return;
    /*
    va_list ap;
    va_start(ap, fmt);
    char buf[1000];
    sprintf(buf,fmt,ap);
    scr_puts(buf);
    va_end(ap);
    return;
    */
    
    u64int i;
    int ch;
    char *s;
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        switch (*fmt) {
            case '%':
                fmt++;
                switch (*fmt) {
                    case '%':
                        scr_putch(*fmt);
                        fmt++;
                        break;
                    case 'c':
                        ch = va_arg(ap, int);
                        scr_putch(ch);
                        fmt++;
                        break;
                    case 's':
                        s = va_arg(ap, char*);
                        scr_puts(s);
                        fmt++;
                        break;
                    case 'd':
                        i = va_arg(ap, u64int);
                        scr_putn(i);
                        fmt++;
                        break;
                    case 'p':
                    case 'x':
                        scr_puts("0x");
                        i = va_arg(ap, u64int);
                        scr_puthex(i);
                        fmt++;
                        break;
                    default:
                        scr_putch('%');
                        scr_putch(*fmt);
                        fmt++;
                        break;
                }
                break;
            case '\\':
                fmt++;
                switch (*fmt) {
                    case '\\':
                        scr_putch('\\');
                        fmt++;
                        break;
                    case '"':
                        scr_putch('"');
                        fmt++;
                        break;
                    case 'b':
                        scr_putch('\b');
                        fmt++;
                        break;
                    case 't':
                        scr_putch('\t');
                        fmt++;
                        break;
                    case 'n':
                        scr_putch('\n');
                        fmt++;
                        break;
                    case 'r':
                        scr_putch('\r');
                        fmt++;
                        break;
                    default:
                        scr_putch('\\');
                        scr_putch(*fmt);
                        fmt++;
                        break;
                }
            default:
                scr_putch(*fmt++);
        }
    }

    va_end(ap);
}

char *dirname(char *dest, const char *s)
{
    strcpy(dest,s);

    if (strcmp(dest, "/") == 0) 
        return dest;

    char *slash = strrchr(dest, '/');

    if (slash > dest) {
        *slash = 0;
    } else {
        strcpy(dest, "/");
    }

    return dest;
}

char *basename(char *dest, const char *s)
{
    strcpy(dest,s);
    u64int i;

    char *slash = strrchr(s, '/');

    if (slash) {
        slash++;
        i = 0;
        while (slash[i]) {
            dest[i] = slash[i];
            i++;
        }
        dest[i] = 0;
    } else 
        strcpy(dest,s);

    return dest;
}

