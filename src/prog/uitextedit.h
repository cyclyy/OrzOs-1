#ifndef UITEXTEDIT_H
#define UITEXTEDIT_H

#include "uidef.h"

#define OZUI_WIDGET_TYPE_TEXT_EDIT          3
#define OZUI_TEXT_EDIT_DEFAULT_FONT_SIZE    12

struct OzUIWindow;
struct OzUIWidget;

struct OzUITextEdit
{
    struct OzUIWidget *widget;
    int fontSize;
    int cursorPos;
    int allowMultiline;
    wchar_t *text;
    struct OzUITextLayout *layout;
    struct OzUITextEditOperation *ops;
    void *d;
};

struct OzUITextEditOperation
{
    void (*onCreate)(struct OzUITextEdit *textEdit);
    void (*onDestroy)(struct OzUITextEdit *textEdit);
};

struct OzUITextEdit *OzUICreateTextEdit(struct OzUIWindow *window, struct Rect *rect, struct OzUITextEditOperation *ops, void *userData);

void OzUIDestroyTextEdit(struct OzUITextEdit *textEdit);

const wchar_t *OzUITextEditGetText(struct OzUITextEdit *textEdit);

void OzUITextEditSetText(struct OzUITextEdit *textEdit, const wchar_t *text);

int OzUITextEditGetFontSize(struct OzUITextEdit *textEdit);

void OzUITextEditSetFontSize(struct OzUITextEdit *textEdit, int fontSize);

int OzUITextEditGetCursorPos(struct OzUITextEdit *textEdit);

void OzUITextEditSetCursorPos(struct OzUITextEdit *textEdit, int cursorPos);

int OzUITextEditIsAllowMultiline(struct OzUITextEdit *textEdit);

void OzUITextEditSetAllowMultiline(struct OzUITextEdit *textEdit, int allow);

#endif /* UITEXTEDIT_H */
