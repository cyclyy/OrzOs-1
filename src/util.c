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

void memset(void *addr, u8int c, u32int n)
{
    while(n) {
        ((u8int *)addr)[--n] = c;
    }
}

u32int vsprintf(char *buf, char *fmt, va_list ap)
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

u32int sprintf(char *buf, char *fmt, ...)
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
    
    u64int i,j;
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

