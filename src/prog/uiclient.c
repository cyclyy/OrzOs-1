#include "uiproto.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uitextedit.h"
#include "uiimagewidget.h"
#include "utf8.h"
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
    char s[100];
    wcstous(s, OzUITextEditGetText(textEdit), 100);
    OzNewTask(s, 0);
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

    window = OzUICreateWindow(200,140,0);

    /*
    tlc.fontSize = 12;
    tlc.rect.x = 0;
    tlc.rect.y = 0;
    tlc.rect.w = 80;
    tlc.rect.h = 20;
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
    */

    label = OzUICreateLabel(window, makeRect(10, 10, 40, 20));
    OzUILabelSetText(label, L"路径：");

    textEdit = OzUICreateTextEdit(window, makeRect(60,10,130,20), &myTextEditOps, 0);
    //OzUITextEditSetText(textEdit, L"");
    OzUITextEditSetAllowMultiline(textEdit, 1);

    button = OzUICreateButton(window, makeRect(50,60,90,40), &myBtnOps, 0);
    OzUIButtonSetText(button, L"启动");

    OzUINextEvent();
    for (;;) {
        OzReceive(&hdr, buf, 512);
        OzUIDispatchEvent(buf);
        OzUINextEvent();
    }
    for (;;);
    OzUIDestroyWindow(window);
    return 0;
}
