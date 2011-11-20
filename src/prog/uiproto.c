#include "uiproto.h"
#include "uidef.h"
#include "uiapp.h"
#include "sysdef.h"
#include "syscall.h"
#include "event.h"
#include "mice.h"
#include "libc/list.h"
#include <string.h>
#include <stdlib.h>

void OzUIWindowOnMiceEvent(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent);

struct OzUIWindowOperation genericWindowOperation = {
    .onMiceEvent = &OzUIWindowOnMiceEvent,
};

static int OzUISendReceive(void *request, void *reply)
{
    struct MessageHeader hdr;
    hdr.pid = UISERVER_PID;
    switch (GET_EVENT_TYPE(request)) {
        case OZUI_EVENT_CREATE_WINDOW:
            return OzSendReceive(&hdr, request, sizeof(struct OzUICreateWindowRequest), reply, sizeof(struct OzUICreateWindowReply));
            break;
        case OZUI_EVENT_DESTROY_WINDOW:
            return OzSendReceive(&hdr, request, sizeof(struct OzUIDestroyWindowRequest), reply, sizeof(struct OzUIDestroyWindowReply));
            break;
        case OZUI_EVENT_MOVE_WINDOW:
            return OzSendReceive(&hdr, request, sizeof(struct OzUIMoveWindowRequest), reply, sizeof(struct OzUIMoveWindowReply));
            break;
        case OZUI_EVENT_DRAW_RECTANGLE:
            return OzSendReceive(&hdr, request, sizeof(struct OzUIWindowDrawRectangleRequest), reply, sizeof(struct OzUIWindowDrawRectangleReply));
            break;
        case OZUI_EVENT_DRAW_TEXT:
            return OzSendReceive(&hdr, request, SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST(request), reply, SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY_FOR_TEXT(((struct OzUIWindowDrawTextRequest*)request)->text));
            break;
    };
    return 0;
}

static int OzUISend(void *request)
{
    switch (GET_EVENT_TYPE(request)) {
        case OZUI_EVENT_NEXT_EVENT:
            return OzSend(UISERVER_PID, request, sizeof(struct OzUINextEventRequest));
            break;
    };
    return 0;
}

static struct OzUIWidget *widgetUnderMice(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent, struct OzUIMiceEvent *localMiceEvent)
{
    struct OzUIWidget *widget;
    listForEachEntry(widget, &window->widgetList, link) {
        if (insideRect(&widget->rect, miceEvent->x, miceEvent->y)) {
            if (localMiceEvent) {
                memcpy(localMiceEvent, miceEvent, sizeof(struct MiceEvent));
                localMiceEvent->x -= widget->rect.x;
                localMiceEvent->y -= widget->rect.y;
            }
            return widget;
        }
    }
    return 0;
}

void OzUIWindowOnMiceEvent(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent)
{
    struct OzUIWidget *widget;
    struct OzUIMiceEvent localMiceEvent;
    widget = widgetUnderMice(window, miceEvent, &localMiceEvent);
    if (widget) {
        if (window->miceWidget != widget) {
            // TODO: handle mice leave in old miceWidget
            if (window->miceWidget) {
                localMiceEvent.type = OZUI_EVENT_MICE_LEAVE;
                if (window->miceWidget->ops && window->miceWidget->ops->onMiceLeave)
                    window->miceWidget->ops->onMiceLeave(window->miceWidget);
            }

            // handle mice enter event
            localMiceEvent.type = OZUI_EVENT_MICE_ENTER;
            if (widget->ops && widget->ops->onMiceEnter)
                widget->ops->onMiceEnter(widget);
        }
        localMiceEvent.x = miceEvent->x - widget->rect.x;
        localMiceEvent.y = miceEvent->y - widget->rect.y;
        if (widget->ops && widget->ops->onMiceEvent)
            widget->ops->onMiceEvent(widget, &localMiceEvent);
    } else if (window->miceWidget) {
        localMiceEvent.type = OZUI_EVENT_MICE_LEAVE;
        if (window->miceWidget->ops && window->miceWidget->ops->onMiceLeave)
            window->miceWidget->ops->onMiceLeave(window->miceWidget);
    }
    window->miceWidget = widget;
}

struct OzUIWindow *OzUIGetWindowById(unsigned long id)
{
    struct OzUIWindow *window;
    listForEachEntry(window, &app->windowList, link) {
        if (window->id == id)
            return window;
    }
    return 0;
}

struct OzUIWindow *OzUICreateWindow(int w, int h, int flags)
{
    struct OzUIWindow *window;
    struct OzUICreateWindowRequest request;
    struct OzUICreateWindowReply reply;
    request.type = OZUI_EVENT_CREATE_WINDOW;
    request.width = w;
    request.height = h;
    request.flags = flags;
    OzUISendReceive(&request, &reply);
    window = (struct OzUIWindow*)malloc(sizeof(struct OzUIWindow));
    window->id = reply.id;
    window->width = w;
    window->height = h;
    window->ops = &genericWindowOperation;
    INIT_LIST_HEAD(&window->widgetList);
    OzUIAppAddWindow(window);
    if (window->ops && window->ops->onCreate)
        window->ops->onCreate(window);
    return window;
}

int OzUIDestroyWindow(struct OzUIWindow *window)
{
    struct OzUIDestroyWindowRequest request;
    struct OzUIDestroyWindowReply reply;
    if (window->ops && window->ops->onDestroy)
        window->ops->onDestroy(window);
    OzUIAppRemoveWindow(window);
    request.type = OZUI_EVENT_DESTROY_WINDOW;
    request.id = window->id;
    OzUISendReceive(&request, &reply);
    free(window);
    return reply.ret;
}


int OzUIMoveWindow(struct OzUIWindow *window, int x, int y)
{
    struct OzUIMoveWindowRequest request;
    struct OzUIMoveWindowReply reply;
    request.type = OZUI_EVENT_MOVE_WINDOW;
    request.id = window->id;
    request.x = x;
    request.y = y;
    OzUISendReceive(&request, &reply);
    window->screenX = x;
    window->screenY = y;
    return reply.ret;
}

int OzUIDispatchEvent(void *buf)
{
    struct OzUIMiceEventNotify *uiMiceEventNotify;
    struct OzUIWindow *window;
    int isUIEvent = 1;
    switch GET_EVENT_TYPE(buf) {
        case OZUI_EVENT_MICE:
            uiMiceEventNotify = (struct OzUIMiceEventNotify*)buf;
            window = OzUIGetWindowById(uiMiceEventNotify->id);
            if (window && window->ops && window->ops->onMiceEvent)
                window->ops->onMiceEvent(window, &uiMiceEventNotify->miceEvent);
            break;
        default:
            isUIEvent = 0;
    };
    return isUIEvent;
}

void OzUINextEvent()
{
    struct OzUINextEventRequest request;
    request.type = OZUI_EVENT_NEXT_EVENT;
    OzUISend(&request);
}

int OzUIWindowDrawRectangle(struct OzUIWindow *window, struct Rect *clipRect,
        struct Rect *rect, struct LineStyle *lineStyle, struct FillStyle *fillStyle)
{
    struct OzUIWindowDrawRectangleRequest request;
    struct OzUIWindowDrawRectangleReply reply;
    request.type = OZUI_EVENT_DRAW_RECTANGLE;
    request.id = window->id;
    memcpy(&request.clipRect, clipRect, sizeof(struct Rect));
    memcpy(&request.rect, rect, sizeof(struct Rect));
    memcpy(&request.lineStyle, lineStyle, sizeof(struct LineStyle));
    memcpy(&request.fillStyle, fillStyle, sizeof(struct FillStyle));
    OzUISendReceive(&request, &reply);
    return reply.ret;
}

int OzUIWindowDrawText(struct OzUIWindow *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, 
        const wchar_t *text, 
        struct LineStyle *lineStyle, 
        struct OzUITextLayout *layout)
{
    struct OzUIWindowDrawTextRequest *request;
    struct OzUIWindowDrawTextReply *reply;
    int i,j;
    i = SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST_FOR_TEXT(text);
    j = SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY_FOR_TEXT(text);
    request = (struct OzUIWindowDrawTextRequest*)malloc(SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST_FOR_TEXT(text));
    reply = (struct OzUIWindowDrawTextReply*)malloc(SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY_FOR_TEXT(text));
    request->type = OZUI_EVENT_DRAW_TEXT;
    request->id = window->id;
    memcpy(&request->clipRect, clipRect, sizeof(struct Rect));
    memcpy(&request->tlc, tlc, sizeof(struct OzUITextLayoutConstraint));
    wcscpy(request->text, text);
    memcpy(&request->lineStyle, lineStyle, sizeof(struct LineStyle));
    OzUISendReceive(request, reply);
    if (layout)
        memcpy(layout, &reply->layout, SIZE_OZUI_TEXT_LAYOUT(&reply->layout));
    free(request);
    free(reply);
    return reply->ret;
}

struct OzUIWidget *OzUICreateWidget(struct OzUIWindow *window, int type, struct Rect *rect, struct OzUIWidgetOperation *ops, void *userData)
{
    struct OzUIWidget *widget;
    widget = (struct OzUIWidget*)malloc(sizeof(struct OzUIWidget));
    memset(widget, 0, sizeof(struct OzUIWidget));
    widget->window = window;
    widget->type = type;
    memcpy(&widget->rect, rect, sizeof(struct Rect));
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

int OzUIWidgetInvalidate(struct OzUIWidget *widget, const struct Rect *dirtyRect)
{
    OzUIWidgetBeginDraw(widget, dirtyRect);
    doWidgetPaint(widget);
    OzUIWidgetEndDraw(widget);
    return 0;
}

int OzUIWidgetInvalidateAll(struct OzUIWidget *widget)
{
    struct Rect dirtyRect;

    initRect(&dirtyRect, 0, 0, widget->rect.w, widget->rect.h);
    return OzUIWidgetInvalidate(widget, &dirtyRect);
}

int OzUIWidgetDrawRectangle(struct OzUIWidget *widget, struct Rect *rect,
        struct LineStyle *lineStyle, struct FillStyle *fillStyle)
{
    struct Rect baseRect;
    memcpy(&baseRect, rect, sizeof(struct Rect));
    baseRect.x += widget->rect.x;
    baseRect.y += widget->rect.y;
    return OzUIWindowDrawRectangle(widget->window, &widget->dirtyRect, &baseRect, lineStyle, fillStyle);
}

void OzUIWidgetSetUserData(struct OzUIWidget *widget, void *userData)
{
    widget->d = userData;
}

void *OzUIWidgetGetUserData(struct OzUIWidget *widget)
{
    return widget->d;
}

static void translateLayoutCoords(struct OzUITextLayout *layout, int deltaX, int deltaY) 
{
    int i;
    translateRect(&layout->rect, deltaX, deltaY);
    for (i = 0; i <= layout->chars; i++) {
        translateRect(&layout->charLayout[i].rect, deltaX, deltaY);
    }
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
// vim: sw=4 sts=4 et tw=100
