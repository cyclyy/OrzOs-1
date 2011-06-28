#include "syscall.h"

int main()
{
    char *s = "Init process running.\n";
    char *p;

    p = s;
    while (*p) {
        OzPutChar(*p);
        p++;
    }

    OzNewTask("Boot:/server",0);

    while(1);
}

