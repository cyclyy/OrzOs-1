#include "syscall.h"

int main()
{
    char *s = "Init process running.\n";
    char *p;

    p = s;
    while (*p) {
        PutChar(*p);
        p++;
    }

    NewTask("Boot:/server",0);

    while(1);
}

