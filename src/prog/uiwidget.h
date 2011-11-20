#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "rect.h"
#include "libc/list.h"

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


#endif /* UIWIDGET_H */
