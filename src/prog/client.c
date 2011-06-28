#include "syscall.h"

int main()
{
    char c = 'x', d;
    s64int client;

    client = OzConnect(100);

    OzPost(client, "a", 1);
    OzPost(client, "b", 1);
    OzPost(client, "c", 1);

    OzSend(client, &c, 1, &d, 1);

    OzPutChar(d);
    OzPutChar('\n');

    for (;;);
}

