#include "screen.h"
#include "common.h"

u16int *video_addr = (u16int *) 0xb8000;
u8int cursor_x, cursor_y;

void scroll()
{
    if (cursor_y >= 25) {
        u32int i,j;

        for (i=0; i<24; i++) {
            for (j=0; j<80; j++)
                video_addr[i * 80 + j] = video_addr[ (i+1)*80 + j];
        }

        // black background color, white text color;
        u8int attrByte = (0 /* black */ << 4) | (15 /* white */ & 0xF);
        u16int blank = 0x20 /* space */ | (attrByte << 8);

        for (j = 0; j<80; j++)
            video_addr[24 * 80 + j] = blank;

        cursor_y = 24;
    }
}

void move_cursor()
{
    u16int cursor_pos = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, cursor_pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, cursor_pos);
}

// clear screen
void scr_clear()
{
    // black background color, white text color;
    u8int attrByte = (0 /* black */ << 4) | (15 /* white */ & 0xF);
    u16int blank = 0x20 /* space */ | (attrByte << 8);

    u16int i;
    for (i=0; i<80*25; i++) {
        video_addr[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 0;

    move_cursor();
}

// put a char
void scr_putch(char c)
{
    // black background color, white text color;
    u8int attrByte = (0 /* black */ << 4) | (15 /* white */ & 0xF);
    u16int attr = attrByte << 8;

    u16int cursor_pos;

    if (c == '\b') {
        if (cursor_x > 0) { 
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = 24;
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8-1);
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\n') { 
        cursor_x = 0;
        cursor_y++;
    } else if (c >= ' ') {
        cursor_pos = cursor_y * 80 + cursor_x;
        video_addr[cursor_pos] = c | attr;
        cursor_x++;

        if (cursor_x>=80) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    scroll();

    move_cursor();
}

// put a string
void scr_puts(const char *s)
{
    if (!s)
        return;
    u16int i = 0;
    while(s[i]) {
        scr_putch(s[i++]);
    }

}


// put a dec num
void scr_putn(u32int i)
{
    if (i==0) {
        scr_putch('0');
    } else {
            
        char s[10];
        int j;
        j = 8;
        s[9] = 0;
        while (i) {
            s[j--] = '0' + (i % 10);
            i /= 10;
        }
        scr_puts(s+j+1);
    }
}

// put a hex num
void scr_puthex(u32int i)
{
    if (i==0) {
        scr_putch('0');
    } else {
        char s[10];
        int j;
        j = 8;
        s[9] = 0;
        while (i) {
            int k;
            k = i % 16;
            if (k < 10)
                s[j--] = '0' + k;
            else
                s[j--] = 'A' + k - 10;
            i /= 16;
        }
        scr_puts(s+j+1);
    }
}

void scr_putp(const char *msg, void *p)
{
    scr_puts(msg);
    scr_puts(": 0x");
    scr_puthex((u32int)p);
    scr_putch('\n');
}

void printk(char *fmt, ...)
{
    if (!fmt) 
        return;

    u32int i,j;
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
                        scr_putch(*fmt);
                        fmt++;
                        break;
                    case 'c':
                        ch = va_arg(ap, char);
                        scr_putch(ch);
                        fmt++;
                        break;
                    case 's':
                        s = va_arg(ap, char*);
                        scr_puts(s);
                        fmt++;
                        break;
                    case 'd':
                        i = va_arg(ap, u32int);
                        scr_putn(i);
                        fmt++;
                        break;
                    case 'p':
                    case 'x':
                        scr_puts("0x");
                        i = va_arg(ap, u32int);
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

