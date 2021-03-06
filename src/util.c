#include "util.h"
#include "debugcon.h"
#include "kmm.h"

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

void insl(u16int port, void *buffer, u32int quads) {
    u32int i;
    for (i=0; i<quads; i++) {
        ((u32int*)buffer)[i] = inl(port);
    }
}

void insw(u16int port, void *buffer, u32int words) {
    u32int i;
    for (i=0; i<words; i++) {
        ((u16int*)buffer)[i] = inw(port);
    }
}

void insb(u16int port, void *buffer, u32int bytes) {
    u32int i;
    for (i=0; i<bytes; i++) {
        ((u8int*)buffer)[i] = inb(port);
    }
}

void outsl(u16int port, void *buffer, u32int quads) {
    u32int i;
    for (i=0; i<quads*4; i+=4) {
        outl(port, ((u32int*)buffer)[i]);
    }
}

void outsw(u16int port, void *buffer, u32int words) {
    u32int i;
    for (i=0; i<words*2; i+=2) {
        outw(port, ((u16int*)buffer)[i]);
    }
}

void outsb(u16int port, void *buffer, u32int bytes) {
    u32int i;
    for (i=0; i<bytes; i++) {
        outb(port, ((u8int*)buffer)[i]);
    }
}

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
    char tmpBuf[MAX_NAME_LEN];

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
                        strcpy(buf+idx, ltoa(i,tmpBuf,10));
                        idx += strlen(tmpBuf);
                        fmt++;
                        break;
                    case 'p':
                    case 'x':
                        //scr_puts("0x");
                        buf[idx++] = '0';
                        buf[idx++] = 'x';
                        i = va_arg(ap, u64int);
                        strcpy(buf+idx, ltoa(i,tmpBuf,16));
                        idx += strlen(tmpBuf);
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

s64int strncmp(const char *s1, const char *s2, u64int n)
{
    u64int i;
    for (i=0; i<n; i++) {
        if (s1[i]<s2[i])
            return -1;
        if (s1[i]>s2[i])
            return 1;
    }
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

void printk(const char *fmt, ...)
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
                        dbgPutch(*fmt);
                        fmt++;
                        break;
                    case 'c':
                        ch = va_arg(ap, int);
                        dbgPutch(ch);
                        fmt++;
                        break;
                    case 's':
                        s = va_arg(ap, char*);
                        dbgPuts(s);
                        fmt++;
                        break;
                    case 'd':
                        i = va_arg(ap, u64int);
                        dbgPutn(i);
                        fmt++;
                        break;
                    case 'p':
                    case 'x':
                        dbgPuts("0x");
                        i = va_arg(ap, u64int);
                        dbgPuthex(i);
                        fmt++;
                        break;
                    default:
                        dbgPutch('%');
                        dbgPutch(*fmt);
                        fmt++;
                        break;
                }
                break;
            case '\\':
                fmt++;
                switch (*fmt) {
                    case '\\':
                        dbgPutch('\\');
                        fmt++;
                        break;
                    case '"':
                        dbgPutch('"');
                        fmt++;
                        break;
                    case 'b':
                        dbgPutch('\b');
                        fmt++;
                        break;
                    case 't':
                        dbgPutch('\t');
                        fmt++;
                        break;
                    case 'n':
                        dbgPutch('\n');
                        dbgPutch('\r');
                        fmt++;
                        break;
                    case 'r':
                        dbgPutch('\r');
                        fmt++;
                        break;
                    default:
                        dbgPutch('\\');
                        dbgPutch(*fmt);
                        fmt++;
                        break;
                }
            default:
                dbgPutch(*fmt++);
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

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c)    (('0' <= (c) && (c) <= '9') \
             || ('a' <= (c) && (c) <= 'f') \
             || ('A' <= (c) && (c) <= 'F'))

#define isdigit(c)    ('0' <= (c) && (c) <= '9')

u64int strtoul(const char *cp,char **endp,u32int base)
{
    u64int result = 0,value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    } else if (base == 16) {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
            cp += 2;
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

s64int strtol(const char *cp,char **endp,u32int base)
{
    if(*cp=='-')
        return -strtoul(cp+1,endp,base);
    return strtoul(cp,endp,base);
}

s32int atoi(const char *nptr)
{
    return strtol(nptr, (char **)0, 10);
}

char *ltoa(s64int num, char *buf, int base)
{
    if (num < 0) {
        buf[0] = '-';
        ultoa(-num, buf+1, base);
    } else {
        ultoa(num, buf, base);
    }
    return buf;
}

char *ultoa(u64int num, char *buf, int base)
{
    u64int n = num, p = 1;
    s64int len = 0, i, x;
    do {
        len++;
        n /= base;
        p *= base;
    } while (n!=0);
    for (i=0; i<len; i++) {
        p /= base;
        x = num / p;
        if ((x>=0) && (x<10)) {
            buf[i] = '0' + x;
        } else if ((x>=10) && (x<16)) {
            buf[i] = 'A' + x - 10;
        } else {
            buf[i] = '0';
        }
    }
    buf[len] = 0;
    return buf;
}

void clearBit(u64int *x, u64int i)
{
    x[i>>6] &= ~(1 << (i & 0x3f));
}

void setBit(u64int *x, u64int i)
{
    x[i>>6] |= 1 << (i & 0x3f);
}

char *strdup(const char *s)
{
    char *ret = (char*)kMalloc(strlen(s) + 1);
    strcpy(ret, s);

    return ret;
}

