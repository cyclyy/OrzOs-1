#include "syscall.h"

int main()
{
    char buf[10];
    s64int server, client;

    server = CreateServer(100);

    PutChar('S');
    PutChar('\n');

    NewTask("Boot:/client",0);

    while (1) {
        client = Receive(server, buf, 10);
        buf[0] -= 32;
        if (client) {
            Reply(client, buf, 1);
        } else {
            PutChar(buf[0]);
            PutChar('\n');
        }
    }
}

