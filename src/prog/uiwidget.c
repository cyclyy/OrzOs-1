#include "uiwidget.h"
#include "uiwindow.h"
#include "uikey.h"
#include "uimice.h"
#include "point.h"
#include <stdlib.h>
#include <string.h>
#include <os/list.h>

struct OzUIWidget *OzUICreateWidget(struct OzUIWindow *window, int type, int flags, struct Rect *rect, struct OzUIWidgetOperation *ops, void *userData)
{
    struct OzUIWidget *widget;
    widget = (struct OzUIWidget*)malloc(sizeof(struct OzUIWidget));
    memset(widget, 0, sizeof(struct OzUIWidget));
    widget->window = window;
    widget->type = type;
    widget->flags = flags;
    copyRect(&widget->rect, rect);
    translateRect(&widget->rect, widget->window->clientRect.x, widget->window->clientRect.y);
    initRect(&widget->dirtyRect, 0, 0, 0, 0);
    widget->ops = ops;
    widget->d = userData;
    listAdd(&widget->link, &window->widgetList);
    if (widget->ops && widget->ops->onCreate)
        widget->ops->onCreate(widget);
    return widget;
}

int OzUIDestroyWidget(struct OzUIWidget *widget)
{
    listDel(&widget->link);
    if (widget->ops && widget->ops->onDestroy)
        widget->ops->onDestroy(widget);
    if (widget->window->miceWidget == widget)
        widget->window->miceWidget = 0;
    if (widget->window->focusWidget == widget)
        widget->window->focusWidget = 0;
    free(widget);
    return 0;
}

int OzUIWidgetBeginDraw(struct OzUIWidget *widget, const struct Rect *dirtyRect)
{
    copyRect(&widget->dirtyRect, dirtyRect);
    translateRect(&widget->dirtyRect, widget->rect.x, widget->rect.y);
    crossRect(&widget->dirtyRect, &widget->rect);
    return 0;
}

int OzUIWidgetEndDraw(struct OzUIWidget *widget)
{
    initRect(&widget->dirtyRect, 0, 0, 0, 0);
    return 0;
}

static void doWidgetPaint(struct OzUIWidget *widget)
{
    if (isEmptyRect(&widget->dirtyRect))
        return;
    if (!widget->ops || !widget->ops->paint)
        return;

    widget->ops->paint(widget);
}

void OzUIWidgetInvalidate(struct OzUIWidget *widget, const struct Rect *dirtyRect)
{
    OzUIWidgetBeginDraw(widget, dirtyRect);
    doWidgetPaint(widget);
    OzUIWidgetEndDraw(widget);
}

void OzUIWidgetInvalidateAll(struct OzUIWidget *widget)
{
    struct Rect dirtyRect;

    initRect(&dirtyRect, 0, 0, widget->rect.w, widget->rect.h);
    OzUIWidgetInvalidate(widget, &dirtyRect);
}

int OzUIWidgetDrawRectangle(struct OzUIWidget *widget, struct Rect *rect,
        struct LineStyle *lineStyle, struct FillStyle *fillStyle)
{
    struct Rect baseRect;
    copyRect(&baseRect, rect);
    translateRect(&baseRect, widget->rect.x, widget->rect.y);
    return OzUIWindowDrawRectangle(widget->window, &widget->dirtyRect, &baseRect, lineStyle, fillStyle);
}

int OzUIWidgetDrawLine(struct OzUIWidget *widget,
        struct Point *p1, struct Point *p2, struct LineStyle *lineStyle)
{
    struct Point baseP1, baseP2;
    baseP1.x = p1->x + widget->rect.x;
    baseP1.y = p1->y + widget->rect.y;
    baseP2.x = p2->x + widget->rect.x;
    baseP2.y = p2->y + widget->rect.y;
    return OzUIWindowDrawLine(widget->window, &widget->dirtyRect, &baseP1, &baseP2, lineStyle);
}

static void translateLayoutCoords(struct OzUITextLayout *layout, int deltaX, int deltaY) 
{
    int i;
    translateRect(&layout->rect, deltaX, deltaY);
    for (i = 0; i < layout->chars; i++) {
        translateRect(&layout->charLayout[i].rect, deltaX, deltaY);
    }
}

static struct OzUITextLayout *copyLayout(struct OzUITextLayout *layout)
{
    struct OzUITextLayout *utl;
    int n;
    n = SIZE_OZUI_TEXT_LAYOUT(layout);
    utl = (struct OzUITextLayout*)malloc(n);
    memcpy(utl, layout, n);
    return utl;
}

int OzUIWidgetDrawText(struct OzUIWidget *widget, struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct LineStyle *lineStyle, struct OzUITextLayout *layout)
{
    struct OzUITextLayoutConstraint baseTLC;
    int ret;
    memcpy(&baseTLC, tlc, sizeof(struct OzUITextLayoutConstraint));
    baseTLC.originX += widget->rect.x;
    baseTLC.originY += widget->rect.y;
    baseTLC.rect.x += widget->rect.x;
    baseTLC.rect.y += widget->rect.y;
    ret =  OzUIWindowDrawText(widget->window, &widget->dirtyRect, &baseTLC, text, lineStyle, layout);
    if (layout)
        translateLayoutCoords(layout, -widget->rect.x, -widget->rect.y);
    return ret;
}

int OzUIWidgetQueryTextLayout(struct OzUIWidget *widget, struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct OzUITextLayout *layout)
{
    struct OzUITextLayoutConstraint baseTLC;
    int ret;
    memcpy(&baseTLC, tlc, sizeof(struct OzUITextLayoutConstraint));
    baseTLC.originX += widget->rect.x;
    baseTLC.originY += widget->rect.y;
    baseTLC.rect.x += widget->rect.x;
    baseTLC.rect.y += widget->rect.y;
    ret =  OzUIWindowQueryTextLayout(widget->window, &widget->dirtyRect, &baseTLC, text, layout);
    if (layout)
        translateLayoutCoords(layout, -widget->rect.x, -widget->rect.y);
    return ret;
}

int OzUIWidgetDrawTextLayout(struct OzUIWidget *widget, struct OzUITextLayoutConstraint *tlc, struct LineStyle *lineStyle, struct OzUITextLayout *layout)
{
    struct OzUITextLayoutConstraint baseTLC;
    struct OzUITextLayout *baseLayout;
    int ret;
    memcpy(&baseTLC, tlc, sizeof(struct OzUITextLayoutConstraint));
    baseTLC.originX += widget->rect.x;
    baseTLC.originY += widget->rect.y;
    baseTLC.rect.x += widget->rect.x;
    baseTLC.rect.y += widget->rect.y;
    baseLayout = copyLayout(layout);
    translateLayoutCoords(baseLayout, widget->rect.x, widget->rect.y);
    ret =  OzUIWindowDrawTextLayout(widget->window, &widget->dirtyRect, &baseTLC, lineStyle, baseLayout);
    free(baseLayout);
    return ret;
}

int OzUIWidgetDrawImageFile(struct OzUIWidget *widget, int x, int y, const char *path)
{
    return OzUIWindowDrawImageFile(widget->window, &widget->dirtyRect, x + widget->rect.x, y + widget->rect.y, path);
}

