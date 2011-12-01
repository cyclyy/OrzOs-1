#include "uiproto.h"
#include "uiapp.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uitextedit.h"
#include "uiimagewidget.h"
#include <os/syscall.h>

char replyBuf[3000];
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
wchar_t *msgAbout = L"OrzOs 是一个操作系统。\n"
    "更多关于去看源代码的README。\n"
    "\n"
    "OrzOs is a operating system.\n"
    "To get more information read the source code.\n"
    "\n"
    "Email: wanghoi@126.com\n";

void myBtnMiceLeftClick(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent)
{
    OzUIAppQuit();
}

static struct OzUIButtonOperation myBtnOps = {
    .onMiceLeftClick = &myBtnMiceLeftClick,
};

static struct OzUITextEditOperation myTextEditOps = {
};

void init()
{
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

}

int main()
{
    struct MessageHeader hdr;
    char buf[3000];

    stderr = fopen("Device:/Debug", "w");
    stdout = stderr;

    init();

    window = OzUICreateWindow(200,400,0);

    imageWidget = OzUICreateImageWidget(window, makeRect(10, 10, 180, 90));
    OzUIImageWidgetSetPath(imageWidget, "C:/logo.png");

    button = OzUICreateButton(window, makeRect(60,340,80,30), &myBtnOps, 0);
    OzUIButtonSetText(button, L"关闭");

    label = OzUICreateLabel(window, makeRect(10, 110, 180, 220));
    OzUILabelSetText(label, msgAbout);

    /*
    textEdit = OzUICreateTextEdit(window, makeRect(10,60,80,40), &myTextEditOps, 0);
    OzUITextEditSetText(textEdit, L"我们都是好人对不对啊对不对");
    OzUITextEditSetAllowMultiline(textEdit, 1);
    */

    OzUINextEvent();
    for (;;) {
        OzReceive(&hdr, buf, 3000);
        OzUIDispatchEvent(buf);
        OzUINextEvent();
    }
    return 0;
}
