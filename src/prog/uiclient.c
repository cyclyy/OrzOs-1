#include "uidef.h"
#include "uiproto.h"
#include "syscall.h"

int main()
{
    struct OzUIWindow *win;
    struct MessageHeader hdr;
    char buf[512];
    win = OzUICreateWindow(400,400,0);
    /*
    for (i = 0; i < 5; i++) {
        OzMilliSleep(100);
        OzUIMoveWindow(win, i*10, i*10);
    }
    */
    OzUINextEvent();
    for (;;) {
        OzReceive(&hdr, buf, 512);
        if (OzUIDispatchEvent(buf))
            OzUINextEvent();
    }
    for (;;);
    OzUIDestroyWindow(win);
    return 0;
}
