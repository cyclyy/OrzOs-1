#ifndef UIWINDOW_H
#define UIWINDOW_H

#include "uidef.h"
#include "uigraphic.h"
#include "uitextlayout.h"
#include "point.h"
#include <os/list.h>
#include <wchar.h>

struct OzUIWindow;
struct OzUIWidget;
struct OzUIKeyEvent;
struct OzUIMiceEvent;
struct OzUILabel;
struct OzUIButton;

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
    struct Rect screenRect;
    int flags;
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

struct OzUIWindowDrawLineRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect;
    struct Point p1, p2;
    struct LineStyle lineStyle;
}__attribute__((packed));

struct OzUIWindowDrawLineReply
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
        wcslen(str)+1))
#define SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY(x) \
    (sizeof(int)  + SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS( \
        ((struct OzUIWindowDrawTextReply*)(x))->layout.chars ))
struct OzUIWindowDrawTextReply
{
    int ret;
    struct OzUITextLayout layout;
}__attribute__((packed));

#define SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REQUEST_FOR_TEXT(text) \
    (sizeof(struct OzUIWindowQueryTextLayoutRequest) \
    + sizeof(wchar_t) * (wcslen(text) + 1))
#define SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REQUEST(x) \
    SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REQUEST_FOR_TEXT( \
            ((struct OzUIWindowQueryTextLayoutRequest*)(x))->text)
struct OzUIWindowQueryTextLayoutRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect;
    struct OzUITextLayoutConstraint tlc;
    wchar_t text[0];
}__attribute__((packed));

#define SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REPLY_FOR_TEXT(str) \
    (sizeof(int)  + SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS( \
        wcslen(str)+1))
#define SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REPLY(x) \
    (sizeof(int)  + SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS( \
        ((struct OzUIWindowQueryTextLayoutReply*)(x))->layout.chars ))
struct OzUIWindowQueryTextLayoutReply
{
    int ret;
    struct OzUITextLayout layout;
}__attribute__((packed));

#define SIZE_OZUI_WINDOW_DRAW_TEXT_LAYOUT_REQUEST_FOR_TEXT(text) \
    SIZE_OZUI_WINDOW_DRAW_TEXT_LAYOUT_REQUEST_FOR_CHARS(wcslen(text)+1)
#define SIZE_OZUI_WINDOW_DRAW_TEXT_LAYOUT_REQUEST_FOR_CHARS(n) \
    (sizeof(struct OzUIWindowDrawTextLayoutRequest) \
      - sizeof(struct OzUITextLayout) \
      + SIZE_OZUI_TEXT_LAYOUT_FOR_CHARS(n))
#define SIZE_OZUI_WINDOW_DRAW_TEXT_LAYOUT_REQUEST(x) \
    (sizeof(struct OzUIWindowDrawTextLayoutRequest) \
      - sizeof(struct OzUITextLayout) \
      + SIZE_OZUI_TEXT_LAYOUT( \
            &((struct OzUIWindowDrawTextLayoutRequest*)(x))->layout))
struct OzUIWindowDrawTextLayoutRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect;
    struct OzUITextLayoutConstraint tlc;
    struct LineStyle lineStyle;
    struct OzUITextLayout layout;
}__attribute__((packed));

struct OzUIWindowDrawTextLayoutReply
{
    int ret;
}__attribute__((packed));

struct OzUIWindowDrawImageFileRequest
{
    int type;
    unsigned long id;
    struct Rect clipRect;
    int x, y;
    char path[MAX_PATH_LEN];
}__attribute__((packed));

struct OzUIWindowDrawImageFileReply
{
    int ret;
}__attribute__((packed));

// event notify
struct OzUIFocusEventNotify
{
    int type;
    unsigned long id;
}__attribute__((packed));

struct OzUIWindowOperation
{
    void (*onKeyEvent)(struct OzUIWindow *window, struct OzUIKeyEvent *event);
    void (*onMiceEvent)(struct OzUIWindow *window, struct OzUIMiceEvent *event);
    void (*onCreate)(struct OzUIWindow *window);
    void (*onDestroy)(struct OzUIWindow *window);
    void (*onFocus)(struct OzUIWindow *window);
    void (*onUnfocus)(struct OzUIWindow *window);
};

struct OzUIWindow
{
    unsigned long id;
    struct Rect screenRect, clientRect;
    struct OzUIWindowOperation *ops;
    struct ListHead widgetList;
    struct ListHead link;
    // private members
    struct OzUILabel *titleLabel;
    struct OzUIButton *closeButton;
    struct OzUIWidget *miceWidget, *focusWidget;
    int inDrag;
};

struct OzUIWindow *OzUIGetWindowById(unsigned long id);

struct OzUIWindow *OzUICreateWindow(int w, int h, int flags);

int OzUIDestroyWindow(struct OzUIWindow *window);

int OzUIMoveWindow(struct OzUIWindow *window, int x, int y);

int OzUIWindowDrawRectangle(struct OzUIWindow *window, struct Rect *clipRect,
        struct Rect *rect, struct LineStyle *lineStyle, struct FillStyle *fillStyle);

int OzUIWindowDrawLine(struct OzUIWindow *window, struct Rect *clipRect,
        struct Point *p1, struct Point *p2, struct LineStyle *lineStyle);

int OzUIWindowDrawText(struct OzUIWindow *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct LineStyle *lineStyle, struct OzUITextLayout *layout);

int OzUIWindowQueryTextLayout(struct OzUIWindow *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct OzUITextLayout *layout);

int OzUIWindowDrawTextLayout(struct OzUIWindow *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, struct LineStyle *lineStyle, struct OzUITextLayout *layout);

int OzUIWindowDrawImageFile(struct OzUIWindow *window, struct Rect *clipRect,
        int x, int y, const char *path);
        
#endif /* UIWINDOW_H */
