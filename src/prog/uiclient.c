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
wchar_t *msgIn = L"在里面！";
wchar_t *msgOut = L"在外面！";

void onMyMiceEnter(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = 100;
    rect.h = 40;
    /*
    fs.color.r = 255;
    fs.color.g = 0;
    fs.color.b = 0;
    OzUIWidgetBeginDraw(widget, &rect);
    OzUIWidgetDrawRectangle(widget, &rect, &ls, &fs);
    OzUIWidgetDrawText(widget, &tlc, L"里面!", &ls, layout);
    OzUIWidgetEndDraw(widget);
    */
    widget->d = msgIn;
    OzUIWidgetInvalidate(widget, &rect);
}

void onMyMiceLeave(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = 100;
    rect.h = 40;
    /*
    fs.color.r = 0;
    fs.color.g = 255;
    fs.color.b = 0;
    OzUIWidgetBeginDraw(widget, &rect);
    OzUIWidgetDrawRectangle(widget, &rect, &ls, &fs);
    OzUIWidgetDrawText(widget, &tlc, L"外面!", &ls, layout);
    OzUIWidgetEndDraw(widget);
    */
    widget->d = msgOut;
    OzUIWidgetInvalidate(widget, &rect);
}

void myPaint(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = 100;
    rect.h = 40;
    fs.color.r = 0;
    fs.color.g = 255;
    fs.color.b = 0;
    OzUIWidgetDrawRectangle(widget, &rect, &ls, &fs);
    OzUIWidgetDrawText(widget, &tlc, (const wchar_t*)widget->d, &ls, layout);
}

struct OzUIWidgetOperation myOps = {
    .onCreate = &onMyMiceLeave,
    .onMiceEnter = &onMyMiceEnter,
    .onMiceLeave = &onMyMiceLeave,
    .paint = &myPaint,
};

int main()
{
    struct MessageHeader hdr;
    char buf[512];

    window = OzUICreateWindow(400,400,0);

    tlc.fontSize = 12;
    tlc.rect.x = 0;
    tlc.rect.y = 0;
    tlc.rect.w = 100;
    tlc.rect.h = 40;
    tlc.originX = 0;
    tlc.originY = 0;
    tlc.flags = OZUI_TEXT_ALIGN_CENTER | OZUI_TEXT_ALIGN_VCENTER;

    rect.x = 60;
    rect.y = 60;
    rect.w = 100;
    rect.h = 40;

    ls.color.r = ls.color.g = ls.color.b = 0;
    ls.lineWidth = 1;

    fs.color.r = fs.color.g = fs.color.b = 0;

    widget = OzUICreateWidget(window, 1, &rect, &myOps);
    widget->d = msgOut;
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
