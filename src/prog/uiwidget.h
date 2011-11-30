#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "uidef.h"
#include <os/list.h>
#include <wchar.h>

struct OzUIKeyEvent;
struct OzUIMiceEvent;
struct OzUIWidget;
struct Point;
struct OzUIWidgetOperation
{
    void (*onCreate)(struct OzUIWidget *widget);
    void (*onDestroy)(struct OzUIWidget *widget);
    void (*onMiceEnter)(struct OzUIWidget *widget);
    void (*onMiceLeave)(struct OzUIWidget *widget);
    void (*onMiceEvent)(struct OzUIWidget *widget, struct OzUIMiceEvent *miceEvent);
    void (*onFocus)(struct OzUIWidget *widget);
    void (*onUnfocus)(struct OzUIWidget *widget);
    void (*onKeyEvent)(struct OzUIWidget *widget, struct OzUIKeyEvent *keyEvent);
    void (*paint)(struct OzUIWidget *widget);
};

#define OZUI_WIDGET_FLAG_FOCUSABLE  1

struct OzUIWidget
{
    struct OzUIWindow *window;
    int type;
    int flags;
    struct Rect rect;
    struct Rect dirtyRect;
    struct OzUIWidgetOperation *ops;
    struct ListHead link;
    void *d;
};

struct OzUIWidget *OzUICreateWidget(struct OzUIWindow *window, int type, int flags, struct Rect *rect, struct OzUIWidgetOperation *ops, void *userData);

int OzUIDestroyWidget(struct OzUIWidget *widget);

int OzUIWidgetBeginDraw(struct OzUIWidget *widget, const struct Rect *dirtyRect);

int OzUIWidgetEndDraw(struct OzUIWidget *widget);

void OzUIWidgetInvalidateAll(struct OzUIWidget *widget);

void OzUIWidgetInvalidate(struct OzUIWidget *widget, const struct Rect *dirtyRect);

void OzUIWidgetSetUserData(struct OzUIWidget *widget, void *userData);

void *OzUIWidgetGetUserData(struct OzUIWidget *widget);

int OzUIWidgetDrawRectangle(struct OzUIWidget *widget, struct Rect *rect,
        struct LineStyle *lineStyle, struct FillStyle *fillStyle);

int OzUIWidgetDrawLine(struct OzUIWidget *widget,
        struct Point *p1, struct Point *p2, struct LineStyle *lineStyle);

int OzUIWidgetDrawText(struct OzUIWidget *widget, 
        struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct LineStyle *lineStyle, struct OzUITextLayout *layout);

int OzUIWidgetQueryTextLayout(struct OzUIWidget *widget, struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct OzUITextLayout *layout);

int OzUIWidgetDrawTextLayout(struct OzUIWidget *widget, struct OzUITextLayoutConstraint *tlc, struct LineStyle *lineStyle, struct OzUITextLayout *layout);

int OzUIWidgetDrawImageFile(struct OzUIWidget *widget, int x, int y, const char *path);

#endif /* UIWIDGET_H */
