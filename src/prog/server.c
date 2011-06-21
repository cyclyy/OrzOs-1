#include "syscall.h"

int main()
{
    char buf[10];
    char c = 'M';
    s64int server, client;

    server = CreateServer(100);

    PutChar('S');
    PutChar('\n');

    NewTask("Boot:/client",0);

    while (1) {
        client = Receive(server, buf, 10);
        buf[0] -= 32;
        Reply(client, buf, 1);
    }
}

