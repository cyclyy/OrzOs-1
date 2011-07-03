#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "syscall.h"

int main()
{
    char *s = "Init process running.\n";
    char *p;
    u64int fd;
    u16int ch = 0x0F00 + 'A';

    ch = 0x0F00 + toupper('o');
    fd = OzOpen("Device:/Display",0);
    OzWrite(fd,2,&ch);
    p = s;
    while (*p) {
        OzPutChar(*p);
        p++;
    }

    OzNewTask("Boot:/uiserver",0);

    while(1);
}

