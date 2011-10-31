#include "uidef.h"
#include "uiproto.h"
#include "syscall.h"

int main()
{
    unsigned long id;
    int i;
    id = OzUICreateWindow(400,400,0);
    for (i = 0; i < 10; i++) {
        OzMilliSleep(1000);
        OzUIMoveWindow(id, i*50, i*50);
    }
    for (;;);
}
