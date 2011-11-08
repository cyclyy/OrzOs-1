#ifndef UIPROTO_H
#define UIPROTO_H

#include "sysdef.h"
#include "uidef.h"
#include "rect.h"

struct OzUIWindow *OzUICreateWindow(int w, int h, int flags);

int OzUIDestroyWindow(struct OzUIWindow *window);

int OzUIMoveWindow(struct OzUIWindow *window, int x, int y);

int OzUIReadEvent(struct OzUIWindow *window);

int OzUIDispatchEvent(void *buf);

void OzUINextEvent();

struct OzUIWindow *OzUIGetWindowById(unsigned long id);

int OzUIWindowDrawRectangle(struct OzUIWindow *window, struct Rect *clipRect,
        struct Rect *rect, struct LineStyle *lineStyle, struct FillStyle *fillStyle);

int OzUIWindowDrawText(struct OzUIWindow *window, struct Rect *clipRect,
        struct Rect *rect, char *text, struct LineStyle *lineStyle, struct OzUITextLayout *layout);

// widget related functions
struct OzUIWidget *OzUICreateWidget(struct OzUIWindow *window, int type, struct Rect *rect, struct OzUIWidgetOperation *ops);

int OzUIDestroyWidget(struct OzUIWidget *widget);

int OzUIWidgetDrawRectangle(struct OzUIWidget *widget, struct Rect *rect,
        struct LineStyle *lineStyle, struct FillStyle *fillStyle);

#endif // UIPROTO_H
