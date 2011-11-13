#include "syscall.h"

int main()
{
    char c = 'x', d;
    s64int client;

    client = OzConnect(100);

    OzPost(client, 1, "a");
    OzPost(client, 1, "b");
    OzPost(client, 1, "c");

    OzSend(client, 1, &c, 1,  &d);

    OzPutChar(d);
    OzPutChar('\n');

    for (;;);
}

