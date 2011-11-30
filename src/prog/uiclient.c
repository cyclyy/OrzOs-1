#include "uiproto.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uitextedit.h"
#include "uiimagewidget.h"
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
struct OzUITextEdit *textEdit;
struct OzUIImageWidget *imageWidget;
wchar_t *msgIn = L"在里面！";
wchar_t *msgOut = L"在外面！";

void myBtnMiceLeftClick(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent)
{
    long i = (long)button->d;
    wchar_t s[100];
    i++;
    button->d = (void*)i;
    swprintf(s, 100, L"左点击了%ld次", i);
    OzUIButtonSetText(button, s);
    OzUICreateWindow(200*i,200*i,0);
}

static struct OzUIButtonOperation myBtnOps = {
    .onMiceLeftClick = &myBtnMiceLeftClick,
};

static struct OzUITextEditOperation myTextEditOps = {
};

int main()
{
    struct MessageHeader hdr;
    char buf[512];

    stderr = fopen("Device:/Debug", "w");
    stdout = stderr;

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

    button = OzUICreateButton(window, makeRect(10,10,80,40), &myBtnOps, 0);
    OzUIButtonSetText(button, L"我们都是好人");

    textEdit = OzUICreateTextEdit(window, makeRect(10,60,80,40), &myTextEditOps, 0);
    OzUITextEditSetText(textEdit, L"我们都是好人对不对啊对不对");
    OzUITextEditSetAllowMultiline(textEdit, 1);

    imageWidget = OzUICreateImageWidget(window, makeRect(10, 110, 80, 40));
    OzUIImageWidgetSetPath(imageWidget, "C:/logo.png");

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
