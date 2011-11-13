#include "debugcon.h"
#include "util.h"

u8int dbgPort = 0xe9;
u8int cursor_x, cursor_y;

// put a char
void dbgPutch(char c)
{
    outb(dbgPort, c);
}

// put a string
void dbgPuts(const char *s)
{
    if (!s)
        return;
    u16int i = 0;
    while(s[i]) {
        dbgPutch(s[i++]);
    }

}

// put a dec num
void dbgPutn(u64int i)
{
    if (i==0) {
        dbgPutch('0');
    } else {
        char s[30];
        int j;
        memset(s,0,30);
        j = 28;
        s[9] = 0;
        while (i) {
            s[j--] = '0' + (i % 10);
            i /= 10;
        }
        dbgPuts(s+j+1);
    }
}

// put a hex num
void dbgPuthex(u64int i)
{
    if (i==0) {
        dbgPutch('0');
    } else {
        char s[30];
        int j;
        memset(s,0,30);
        j = 28;
        while (i) {
            int k;
            k = i % 16;
            if (k < 10)
                s[j--] = '0' + k;
            else
                s[j--] = 'A' + k - 10;
            i /= 16;
        }
        dbgPuts(s+j+1);
    }
}

/*
void dbgputp(const char *msg, void *p)
{
    dbgputs(msg);
    dbgputs(": 0x");
    dbgputhex((u32int)p);
    dbgputch('\n');
}
*/

