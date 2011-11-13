#include "syscall.h"

int main()
{
    char buf[10];
    s64int server, client;

    server = OzCreateServer(100);

    OzPutChar('S');
    OzPutChar('\n');

    OzNewTask("Boot:/client",0);

    while (1) {
        client = OzReceive(server, 10, buf, 0);
        buf[0] -= 32;
        if (client) {
            OzReply(client, 1, buf);
        } else {
            OzPutChar(buf[0]);
            OzPutChar('\n');
        }
    }
}

