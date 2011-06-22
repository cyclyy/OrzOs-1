#include "syscall.h"

int main()
{
    char c = 'x', d;
    s64int client;

    client = Connect(100);

    Post(client, "a", 1);
    Post(client, "b", 1);
    Post(client, "c", 1);

    Send(client, &c, 1, &d, 1);

    PutChar(d);
    PutChar('\n');

    for (;;);
}

