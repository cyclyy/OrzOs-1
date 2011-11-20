#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "uidef.h"
#include <os/list.h>
#include <wchar.h>

struct OzUIMiceEvent;
struct OzUIWidget;
struct OzUIWidgetOperation
{
    void (*onCreate)(struct OzUIWidget *widget);
    void (*onDestroy)(struct OzUIWidget *widget);
    void (*onMiceEnter)(struct OzUIWidget *widget);
    void (*onMiceLeave)(struct OzUIWidget *widget);
    void (*onMiceEvent)(struct OzUIWidget *widget, struct OzUIMiceEvent *miceEvent);
    void (*paint)(struct OzUIWidget *widget);
};

struct OzUIWidget
{
    struct OzUIWindow *window;
    int type;
    struct Rect rect;
    struct Rect dirtyRect;
    struct OzUIWidgetOperation *ops;
    struct ListHead link;
    void *d;
};

struct OzUIWidget *OzUICreateWidget(struct OzUIWindow *window, int type, struct Rect *rect, struct OzUIWidgetOperation *ops, void *userData);

int OzUIDestroyWidget(struct OzUIWidget *widget);

int OzUIWidgetBeginDraw(struct OzUIWidget *widget, const struct Rect *dirtyRect);

int OzUIWidgetEndDraw(struct OzUIWidget *widget);

int OzUIWidgetInvalidateAll(struct OzUIWidget *widget);

int OzUIWidgetInvalidate(struct OzUIWidget *widget, const struct Rect *dirtyRect);

void OzUIWidgetSetUserData(struct OzUIWidget *widget, void *userData);

void *OzUIWidgetGetUserData(struct OzUIWidget *widget);

int OzUIWidgetDrawRectangle(struct OzUIWidget *widget, struct Rect *rect,
        struct LineStyle *lineStyle, struct FillStyle *fillStyle);

int OzUIWidgetDrawText(struct OzUIWidget *widget, 
        struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct LineStyle *lineStyle, struct OzUITextLayout *layout);


#endif /* UIWIDGET_H */
