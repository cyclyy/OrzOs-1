#ifndef UIWINDOW_H
#define UIWINDOW_H

#include "uigraphic.h"
#include "uitextlayout.h"
#include <wchar.h>

struct OzUIWindow;
struct OzUIWidget;
struct OzUIMiceEvent;

// format of message need synchronous reply
#define SIZE_OZUI_CREATE_WINDOW_REQUEST(x) \
    (sizeof(struct OzUICreateWindowRequest) \
    + sizeof(wchar_t) * (wcslen(((struct OzUICreateWindowRequest*)(x))->title) + 1))
struct OzUICreateWindowRequest
{
    int type;
    int width, height, flags;
    wchar_t title[0];
}__attribute__((packed));

struct OzUICreateWindowReply
{
    unsigned long id;
    int screenX, screenY, width, height, flags;
    int clientX, clientY, clientWidth, clientHeight;
}__attribute__((packed));

struct OzUIDestroyWindowRequest
{
    int type;
    unsigned long id;
}__attribute__((packed));

struct OzUIDestroyWindowReply
{
    int type;
    int ret;
}__attribute__((packed));

struct OzUIMoveWindowRequest
{
    int type;
    unsigned long id;
    int x, y;
}__attribute__((packed));

struct OzUIMoveWindowReply
{
    int ret;
}__attribute__((packed));

struct OzUIWindowDrawRectangleRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect, rect;
    struct LineStyle lineStyle;
    struct FillStyle fillStyle;
}__attribute__((packed));

struct OzUIWindowDrawRectangleReply
{
    int ret;
}__attribute__((packed));

#define SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST_FOR_TEXT(text) \
    (sizeof(struct OzUIWindowDrawTextRequest) \
    + sizeof(wchar_t) * (wcslen(text) + 1))
#define SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST(x) \
    SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST_FOR_TEXT( \
            ((struct OzUIWindowDrawTextRequest*)(x))->text)
struct OzUIWindowDrawTextRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect;
    struct OzUITextLayoutConstraint tlc;
    struct LineStyle lineStyle;
    wchar_t text[0];
}__attribute__((packed));

#define SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY_FOR_TEXT(str) \
    (sizeof(int)  + SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS( \
        wcslen(str)))
#define SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY(x) \
    (sizeof(int)  + SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS( \
        ((struct OzUIWindowDrawTextReply*)(x))->layout.chars ))
struct OzUIWindowDrawTextReply
{
    int ret;
    struct OzUITextLayout layout;
}__attribute__((packed));

struct OzUIWindowOperation
{
    void (*onMiceEvent)(struct OzUIWindow *window, struct OzUIMiceEvent *event);
};

struct OzUIWindow
{
    unsigned long id;
    int screenX, screenY, width, height;
    struct OzUIWindowOperation *ops;
    struct ListHead widgetList;
    struct ListHead link;
    // private members
    struct OzUIWidget *miceWidget, *focusWidget;
};


#endif /* UIWINDOW_H */
