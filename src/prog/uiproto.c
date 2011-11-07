#include "uiproto.h"
#include "uidef.h"
#include "sysdef.h"
#include "syscall.h"
#include "event.h"
#include "libc/list.h"
#include <string.h>
#include <stdlib.h>

static LIST_HEAD(windowList);

void OzUIWindowOnMiceEvent(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent);

struct OzUIWindowOperation genericWindowOperation = {
    .onMiceEvent = &OzUIWindowOnMiceEvent,
};

static int OzUISendReceive(void *request, void *reply)
{
    struct MessageHeader hdr;
    hdr.pid = UISERVER_PID;
    switch (GET_EVENT_TYPE(request)) {
    case UI_EVENT_CREATE_WINDOW:
        return OzSendReceive(&hdr, request, sizeof(struct OzUICreateWindowRequest), reply, sizeof(struct OzUICreateWindowReply));
        break;
    case UI_EVENT_DESTROY_WINDOW:
        return OzSendReceive(&hdr, request, sizeof(struct OzUIDestroyWindowRequest), reply, sizeof(struct OzUIDestroyWindowReply));
        break;
    case UI_EVENT_MOVE_WINDOW:
        return OzSendReceive(&hdr, request, sizeof(struct OzUIMoveWindowRequest), reply, sizeof(struct OzUIMoveWindowReply));
        break;
    case UI_EVENT_DRAW_RECTANGLE:
        return OzSendReceive(&hdr, request, sizeof(struct OzUIWindowDrawRectangleRequest), reply, sizeof(struct OzUIWindowDrawRectangleReply));
        break;
    };
    return 0;
}

static int OzUISend(void *request)
{
    switch (GET_EVENT_TYPE(request)) {
    case UI_EVENT_NEXT_EVENT:
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
            ;;
            // handle mice enter event
            localMiceEvent.type = UI_EVENT_MICE_ENTER;
            if (widget->ops && widget->ops->onMiceEnter)
                widget->ops->onMiceEnter(widget);
        }

        /*
        localMiceEvent.type = miceEvent.type;
        if (widget->ops && widget->ops->onMiceEvent)
            widget->ops->onMiceEvent(widget, &localMiceEvent);
        */
    } else if (window->miceWidget) {
        localMiceEvent.type = UI_EVENT_MICE_LEAVE;
        if (window->miceWidget->ops && window->miceWidget->ops->onMiceLeave)
            window->miceWidget->ops->onMiceLeave(window->miceWidget);
    }
    window->miceWidget = widget;
}

struct OzUIWindow *OzUIGetWindowById(unsigned long id)
{
    struct OzUIWindow *window;
    listForEachEntry(window, &windowList, link) {
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
    request.type = UI_EVENT_CREATE_WINDOW;
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
    listAdd(&window->link, &windowList);
    return window;
}

int OzUIDestroyWindow(struct OzUIWindow *window)
{
    struct OzUIDestroyWindowRequest request;
    struct OzUIDestroyWindowReply reply;
    request.type = UI_EVENT_DESTROY_WINDOW;
    request.id = window->id;
    OzUISendReceive(&request, &reply);
    free(window);
    return reply.ret;
}


int OzUIMoveWindow(struct OzUIWindow *window, int x, int y)
{
    struct OzUIMoveWindowRequest request;
    struct OzUIMoveWindowReply reply;
    request.type = UI_EVENT_MOVE_WINDOW;
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
    case UI_EVENT_MICE:
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
    request.type = UI_EVENT_NEXT_EVENT;
    OzUISend(&request);
}

int OzUIWindowDrawRectangle(struct OzUIWindow *window, struct Rect *clipRect,
        struct Rect *rect, struct LineStyle *lineStyle, struct FillStyle *fillStyle)
{
    struct OzUIWindowDrawRectangleRequest request;
    struct OzUIWindowDrawRectangleReply reply;
    request.type = UI_EVENT_DRAW_RECTANGLE;
    request.id = window->id;
    memcpy(&request.clipRect, clipRect, sizeof(struct Rect));
    memcpy(&request.rect, rect, sizeof(struct Rect));
    memcpy(&request.lineStyle, lineStyle, sizeof(struct LineStyle));
    memcpy(&request.fillStyle, fillStyle, sizeof(struct FillStyle));
    OzUISendReceive(&request, &reply);
    return reply.ret;
}

struct OzUIWidget *OzUICreateWidget(struct OzUIWindow *window, int type, struct Rect *rect, struct OzUIWidgetOperation *ops)
{
    struct OzUIWidget *widget;
    widget = (struct OzUIWidget*)malloc(sizeof(struct OzUIWidget));
    memset(widget, 0, sizeof(struct OzUIWidget));
    widget->window = window;
    widget->type = type;
    memcpy(&widget->rect, rect, sizeof(struct Rect));
    widget->ops = ops;
    listAdd(&widget->link, &window->widgetList);
    return widget;
}

int OzUIDestroyWidget(struct OzUIWidget *widget)
{
    listDel(&widget->link);
    free(widget);
    return 0;
}

int OzUIWidgetDrawRectangle(struct OzUIWidget *widget, struct Rect *rect,
        struct LineStyle *lineStyle, struct FillStyle *fillStyle)
{
    struct Rect baseRect;
    memcpy(&baseRect, rect, sizeof(struct Rect));
    baseRect.x += widget->rect.x;
    baseRect.y += widget->rect.y;
    return OzUIWindowDrawRectangle(widget->window, &widget->rect, &baseRect, lineStyle, fillStyle);
}
// vim: sw=4 sts=4 et tw=100
