#include "uiproto.h"
#include "uibutton.h"
#include <os/syscall.h>

char replyBuf[1000];
struct OzUITextLayoutConstraint tlc;
struct OzUITextLayout *layout = (struct OzUITextLayout*)replyBuf;
struct Rect rect;
struct LineStyle ls;
struct FillStyle fs;
struct OzUIWindow *window;
struct OzUIWidget *widget;
struct OzUIButton *button;
struct OzUILabel *label;
wchar_t *msgIn = L"在里面！";
wchar_t *msgOut = L"在外面！";

/*
void onMyMiceEnter(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = 100;
    rect.h = 40;
    widget->d = msgIn;
    OzUIWidgetInvalidate(widget, &rect);
}

void onMyMiceLeave(struct OzUIWidget *widget)
{
    struct Rect rect;
    rect.x = rect.y = 0;
    rect.w = 100;
    rect.h = 40;
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
*/

void myBtnMiceLeftClick(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent)
{
    long i = (long)button->d;
    wchar_t s[100];
    i++;
    button->d = (void*)i;
    swprintf(s, 100, L"左点击了%ld次", i);
    OzUIButtonSetText(button, s);
}

static struct OzUIButtonOperation myBtnOps = {
    .onMiceLeftClick = &myBtnMiceLeftClick,
};

int main()
{
    struct MessageHeader hdr;
    char buf[512];

    window = OzUICreateWindow(200,200,0);

    tlc.fontSize = 12;
    tlc.rect.x = 0;
    tlc.rect.y = 0;
    tlc.rect.w = 100;
    tlc.rect.h = 40;
    tlc.originX = 0;
    tlc.originY = 0;
    tlc.flags = OZUI_TEXT_ALIGN_CENTER | OZUI_TEXT_ALIGN_VCENTER;

    rect.x = 0;
    rect.y = 0;
    rect.w = 100;
    rect.h = 40;

    ls.color.r = ls.color.g = ls.color.b = 0;
    ls.lineWidth = 1;

    fs.color.r = fs.color.g = fs.color.b = 0;

    struct Rect buttonRect;
    initRect(&buttonRect, 60, 60, 80, 40);
    button = OzUICreateButton(window, &buttonRect, &myBtnOps, 0);
    OzUIButtonSetText(button, L"我们都是好人");

    /*
    initRect(&buttonRect, 60, 120, 40, 40);
    OzUICreateCloseButton(window, &buttonRect, 0);
    */

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
