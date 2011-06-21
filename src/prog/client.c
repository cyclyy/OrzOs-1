#include "syscall.h"

int main()
{
    char buf[10];
    char c = 'x', d;
    s64int server, client;

    client = Connect(100);

    Send(client, &c, 1, &d, 1);

    PutChar(d);
    PutChar('\n');

    for (;;);
}

