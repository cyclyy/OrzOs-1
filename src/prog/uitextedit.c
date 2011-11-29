#include "uitextedit.h"
#include "uiwindow.h"
#include "uiwidget.h"
#include "uitextlayout.h"
#include "uikey.h"
#include "uimice.h"
#include "utf8.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifndef ABS
#define ABS(x) ((x)>0 ? (x) : -(x))
#endif

static void textEditPaint(struct OzUIWidget *widget);
static void textEditKeyEvent(struct OzUIWidget *widget, struct OzUIKeyEvent *keyEvent);
static void textEditMiceEvent(struct OzUIWidget *widget, struct OzUIMiceEvent *miceEvent);

static struct OzUIWidgetOperation textEditOps = {
    .onKeyEvent = &textEditKeyEvent,
    .onMiceEvent = &textEditMiceEvent,
    .onFocus = &OzUIWidgetInvalidateAll,
    .onUnfocus = &OzUIWidgetInvalidateAll,
    .paint = &textEditPaint,
};

static struct LineStyle blackLS = {
    .color = {0,0,0},
    .lineWidth = 1,
};

static struct FillStyle whiteFS = {
    .color = {255,255,255},
};

static struct OzUITextLayoutConstraint tlc = {
    .originX = 0,
    .originY = 0,
    .fontSize = OZUI_TEXT_EDIT_DEFAULT_FONT_SIZE,
    .flags = OZUI_TEXT_ALIGN_VCENTER,
};

static void textEditPaint(struct OzUIWidget *widget)
{
    struct Rect rect, cursorRect;
    struct OzUITextEdit *textEdit;
    struct Point p1, p2;
    int dx, dy;

    textEdit = (struct OzUITextEdit*)widget->d;
    initRect(&rect, 0, 0, widget->rect.w, widget->rect.h);
    //initRect(&tlc.rect, 0, 0, 100000, 100000);
    OzUIWidgetDrawRectangle(widget, &rect, &blackLS, &whiteFS);
    if (textEdit && textEdit->layout) {
        copyRect(&cursorRect, &textEdit->layout->charLayout[textEdit->cursorPos].rect);
        cursorRect.w = textEdit->fontSize/2;

        dx = dy = 0;
        if (rectLeft(&cursorRect) < rectLeft(&rect))
            dx =  rectLeft(&rect) - rectLeft(&cursorRect);
        if (rectRight(&cursorRect) > rectRight(&rect))
            dx =  rectRight(&rect) - rectRight(&cursorRect);
        if (rectTop(&cursorRect) < rectTop(&rect))
            dy =  rectTop(&rect) - rectTop(&cursorRect);
        if (rectBottom(&cursorRect) > rectBottom(&rect))
            dy =  rectBottom(&rect) - rectBottom(&cursorRect);

        OzUITextLayoutTransform(textEdit->layout, dx, dy);
        translateRect(&cursorRect, dx, dy);
        OzUIWidgetDrawTextLayout(widget, &tlc, &blackLS, textEdit->layout);
        if (widget->window->focusWidget == widget) {
            p1.x = rectLeft(&cursorRect);
            p1.y = rectBottom(&cursorRect);
            p2.x = rectRight(&cursorRect);
            p2.y = rectBottom(&cursorRect);
            OzUIWidgetDrawLine(widget, &p1, &p2, &blackLS);
        }
    }
}

static int findCursorPos(struct OzUITextEdit *textEdit, int x, int y)
{
    int best, bx, by, i, x1, y1;
    best = textEdit->cursorPos;
    bx = rectLeft(&textEdit->layout->charLayout[best].rect);
    by = rectBottom(&textEdit->layout->charLayout[best].rect);
    for (i=0; i<textEdit->layout->chars; i++) {
        x1 = rectLeft(&textEdit->layout->charLayout[i].rect);
        y1 = rectBottom(&textEdit->layout->charLayout[i].rect);
        if ((ABS(by-y) > ABS(y1-y)) || 
                ((ABS(by-y) == ABS(y1-y)) && (ABS(bx-x) > ABS(x1-x)))) {
            best = i;
            bx = x1;
            by = y1;
        }
    }
    return best;
}

static void getCursorRect(struct OzUITextEdit *textEdit, struct Rect *cursorRect)
{
    copyRect(cursorRect, &textEdit->layout->charLayout[textEdit->cursorPos].rect);
    cursorRect->w = textEdit->fontSize/2;
}

static void textEditKeyEvent(struct OzUIWidget *widget, struct OzUIKeyEvent *keyEvent)
{
    struct OzUITextEdit *textEdit;
    struct Rect cursorRect;
    wchar_t *text;
    wchar_t wch;
    textEdit = (struct OzUITextEdit*)widget->d;
    if (keyEvent->type != OZUI_KEY_EVENT_DOWN)
        return;
    switch (keyEvent->code) {
        case Key_Left:
            if (!keyEvent->mask)
                OzUITextEditSetCursorPos(textEdit, textEdit->cursorPos-1);
            break;
        case Key_Right:
            if (!keyEvent->mask)
                OzUITextEditSetCursorPos(textEdit, textEdit->cursorPos+1);
            break;
        case Key_Up:
            if (!keyEvent->mask) {
                getCursorRect(textEdit, &cursorRect);
                OzUITextEditSetCursorPos(textEdit, 
                        findCursorPos(textEdit, rectLeft(&cursorRect), rectBottom(&cursorRect) - textEdit->layout->height));
            }
            break;
        case Key_Down:
            if (!keyEvent->mask) {
                getCursorRect(textEdit, &cursorRect);
                OzUITextEditSetCursorPos(textEdit, 
                        findCursorPos(textEdit, rectLeft(&cursorRect), rectBottom(&cursorRect) + textEdit->layout->height));
            }
            break;
        case Key_Delete:
            if (textEdit->cursorPos < textEdit->layout->chars-1) {
                text = (wchar_t*)calloc(textEdit->layout->chars, sizeof(wchar_t));
                wcsncpy(text, textEdit->text, textEdit->cursorPos);
                wcscpy(&text[textEdit->cursorPos], &textEdit->text[textEdit->cursorPos+1]);
                OzUITextEditSetText(textEdit, text);
                free(text);
            }
            break;
        case Key_Backspace:
            if (textEdit->cursorPos) {
                text = (wchar_t*)calloc(textEdit->layout->chars, sizeof(wchar_t));
                wcsncpy(text, textEdit->text, textEdit->cursorPos-1);
                wcscpy(&text[textEdit->cursorPos-1], &textEdit->text[textEdit->cursorPos]);
                textEdit->cursorPos--;
                OzUITextEditSetText(textEdit, text);
                free(text);
            }
            break;
        case Key_Return:
            if (textEdit->allowMultiline) {
                wch = L'\n';
                text = (wchar_t*)calloc(textEdit->layout->chars+1, sizeof(wchar_t));
                wcsncpy(text, textEdit->text, textEdit->cursorPos);
                text[textEdit->cursorPos] = wch;
                wcscpy(&text[textEdit->cursorPos+1], &textEdit->text[textEdit->cursorPos]);
                textEdit->cursorPos++;
                OzUITextEditSetText(textEdit, text);
                free(text);
            }
            break;
        default:
            wch = OzUIKeyEventTranslate(keyEvent);
            if (wch) {
                text = (wchar_t*)calloc(textEdit->layout->chars+1, sizeof(wchar_t));
                wcsncpy(text, textEdit->text, textEdit->cursorPos);
                text[textEdit->cursorPos] = wch;
                wcscpy(&text[textEdit->cursorPos+1], &textEdit->text[textEdit->cursorPos]);
                textEdit->cursorPos++;
                OzUITextEditSetText(textEdit, text);
                free(text);
            }
    }
}

static void textEditMiceEvent(struct OzUIWidget *widget, struct OzUIMiceEvent *miceEvent)
{
    struct OzUITextEdit *textEdit;
    struct Rect cursorRect;
    if (miceEvent->type == OZUI_MICE_EVENT_DOWN) {
        textEdit = (struct OzUITextEdit*)widget->d;
        getCursorRect(textEdit, &cursorRect);
        OzUITextEditSetCursorPos(textEdit, findCursorPos(textEdit, miceEvent->x, 
                    miceEvent->y + textEdit->layout->height));
    }
}

static void textEditDoLayout(struct OzUITextEdit *textEdit)
{
    free(textEdit->layout);
    textEdit->layout = (struct OzUITextLayout*)malloc(SIZE_OZUI_TEXT_LAYOUT_FOR_TEXT(textEdit->text));
    OzUIWidgetQueryTextLayout(textEdit->widget, &tlc, textEdit->text, textEdit->layout);
}

static void textEditDoSetText(struct OzUITextEdit *textEdit, wchar_t *text)
{
    free(textEdit->text);
    textEdit->text = text;
    textEditDoLayout(textEdit);
    textEdit->cursorPos = MIN(MAX(textEdit->cursorPos, 0), textEdit->layout->chars - 1);
}

struct OzUITextEdit *OzUICreateTextEdit(struct OzUIWindow *window, struct Rect *rect, struct OzUITextEditOperation *ops, void *userData)
{
    struct OzUITextEdit *textEdit;

    textEdit = (struct OzUITextEdit*)malloc(sizeof(struct OzUITextEdit));
    memset(textEdit, 0, sizeof(struct OzUITextEdit));
    textEdit->widget = OzUICreateWidget(window, OZUI_WIDGET_TYPE_TEXT_EDIT, OZUI_WIDGET_FLAG_FOCUSABLE, rect, &textEditOps, textEdit);
    textEdit->fontSize = OZUI_TEXT_EDIT_DEFAULT_FONT_SIZE;
    textEdit->ops = ops;
    textEdit->d = userData;
    initRect(&tlc.rect, 0, 0, 100000, textEdit->widget->rect.h);
    OzUITextEditSetText(textEdit, L"");
    //OzUIWidgetInvalidateAll(textEdit->widget);
    return textEdit;
}

void OzUIDestroyTextEdit(struct OzUITextEdit *textEdit)
{
    OzUIDestroyWidget(textEdit->widget);
    free(textEdit->text);
    free(textEdit);
}

const wchar_t *OzUITextEditGetText(struct OzUITextEdit *textEdit)
{
    return textEdit->text;
}

void OzUITextEditSetText(struct OzUITextEdit *textEdit, const wchar_t *text)
{
    textEditDoSetText(textEdit, wcsdup(text));
    OzUIWidgetInvalidateAll(textEdit->widget);
}

int OzUITextEditGetFontSize(struct OzUITextEdit *textEdit)
{
    return textEdit->fontSize;
}

void OzUITextEditSetFontSize(struct OzUITextEdit *textEdit, int fontSize)
{
    if (textEdit->fontSize == fontSize)
        return;
    textEdit->fontSize = fontSize;
    tlc.fontSize = fontSize;
    OzUIWidgetInvalidateAll(textEdit->widget);
}

int OzUITextEditGetCursorPos(struct OzUITextEdit *textEdit)
{
    return textEdit->cursorPos;
}

void OzUITextEditSetCursorPos(struct OzUITextEdit *textEdit, int cursorPos)
{
    if ((textEdit->cursorPos == cursorPos) || !textEdit->layout)
        return;
    if ((cursorPos < 0) || (cursorPos >= textEdit->layout->chars))
        return;
    textEdit->cursorPos = MIN(MAX(cursorPos, 0), textEdit->layout->chars - 1);
    if (textEdit->widget->window->focusWidget == textEdit->widget)
        OzUIWidgetInvalidateAll(textEdit->widget);
}

int OzUITextEditIsAllowMultiline(struct OzUITextEdit *textEdit)
{
    return textEdit->allowMultiline;
}

void OzUITextEditSetAllowMultiline(struct OzUITextEdit *textEdit, int allow)
{
    if ((allow==0) ^ textEdit->allowMultiline)
        return;
    textEdit->allowMultiline = (allow!=0);

    if (allow) {
        tlc.flags = 0;
        initRect(&tlc.rect, 0, 0, textEdit->widget->rect.w - textEdit->fontSize/2, textEdit->widget->rect.h);
    } else {
        tlc.flags = OZUI_TEXT_ALIGN_VCENTER;
        initRect(&tlc.rect, 0, 0, 100000, textEdit->widget->rect.h);
    }
    textEditDoLayout(textEdit);
    OzUIWidgetInvalidateAll(textEdit->widget);
}
