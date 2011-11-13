#include "uidef.h"
#include "uiproto.h"
#include "syscall.h"

char replyBuf[1000];
struct OzUITextLayoutConstraint tlc;
struct OzUITextLayout *layout = (struct OzUITextLayout*)replyBuf;
struct Rect rect;
struct LineStyle ls;
struct FillStyle fs;
struct OzUIWindow *window;
struct OzUIWidget *widget;

void onMyMiceEnter(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = rect.h = 100;
    fs.color.r = 255;
    fs.color.g = 0;
    fs.color.b = 0;
    OzUIWidgetDrawRectangle(widget, &rect, &ls, &fs);
    OzUIWidgetDrawText(widget, &tlc, L"里面!", &ls, layout);
}

void onMyMiceLeave(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = rect.h = 100;
    fs.color.r = 0;
    fs.color.g = 255;
    fs.color.b = 0;
    OzUIWidgetDrawRectangle(widget, &rect, &ls, &fs);
    OzUIWidgetDrawText(widget, &tlc, L"外面!", &ls, layout);
}

struct OzUIWidgetOperation myOps = {
    .onCreate = &onMyMiceLeave,
    .onMiceEnter = &onMyMiceEnter,
    .onMiceLeave = &onMyMiceLeave,
};

int main()
{
    struct MessageHeader hdr;
    char buf[512];

    window = OzUICreateWindow(400,400,0);

    tlc.fontSize = 20;
    tlc.rect.x = 0;
    tlc.rect.y = 0;
    tlc.rect.w = 100;
    tlc.rect.h = 40;
    tlc.originX = 0;
    tlc.originY = 0;
    tlc.flags = 0;

    rect.x = 60;
    rect.y = 60;
    rect.w = 100;
    rect.h = 40;

    ls.color.r = ls.color.g = ls.color.b = 255;
    ls.lineWidth = 5;

    fs.color.r = fs.color.g = fs.color.b = 0;

    widget = OzUICreateWidget(window, 1, &rect, &myOps);
    OzUINextEvent();
    for (;;) {
        OzReceive(&hdr, buf, 512);
        if (OzUIDispatchEvent(buf))
            OzUINextEvent();
    }
    for (;;);
    OzUIDestroyWindow(window);
    return 0;
}
