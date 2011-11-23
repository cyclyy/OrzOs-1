#include "uiwindow.h"
#include "uiproto.h"
#include "uiwidget.h"
#include "uiapp.h"
#include "uilabel.h"
#include "uiclosebutton.h"
#include <stdlib.h>
#include <string.h>

void OzUIWindowOnMiceEvent(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent);
void OzUIWindowOnFocus(struct OzUIWindow *window);
void OzUIWindowOnUnfocus(struct OzUIWindow *window);

struct OzUIWindowOperation genericWindowOperation = {
    .onMiceEvent = &OzUIWindowOnMiceEvent,
    .onFocus = &OzUIWindowOnFocus,
    .onUnfocus = &OzUIWindowOnUnfocus,
};

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
    static int inDrag = 0;
    static int lastX, lastY;
    if (inDrag) {
        switch (miceEvent->type) {
        case OZUI_MICE_EVENT_DOWN:
        case OZUI_MICE_EVENT_MOVE:
            if (miceEvent->button == OZUI_MICE_BUTTON_LEFT)
                OzUIMoveWindow(window, window->screenRect.x + miceEvent->x - lastX, window->screenRect.y + miceEvent->y - lastY);
            else {
                inDrag = 0;
            }
            break;
        case OZUI_MICE_EVENT_UP:
            OzUIMoveWindow(window, window->screenRect.x + miceEvent->x - lastX, window->screenRect.y + miceEvent->y - lastY);
            inDrag = 0;
            break;
        }
        return;
    }
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
    } else {
        if (window->miceWidget) {
            localMiceEvent.type = OZUI_EVENT_MICE_LEAVE;
            if (window->miceWidget->ops && window->miceWidget->ops->onMiceLeave)
                window->miceWidget->ops->onMiceLeave(window->miceWidget);
        }
        // handle user drag window
        if (!inDrag) {
            if ((miceEvent->type == OZUI_MICE_EVENT_DOWN) && (miceEvent->button == OZUI_MICE_BUTTON_LEFT)) {
                inDrag = 1;
                lastX = miceEvent->x;
                lastY = miceEvent->y;
            }
        } else {
        }
    }
    window->miceWidget = widget;
}

void OzUIWindowOnFocus(struct OzUIWindow *window)
{
    OzUILabelSetText(window->titleLabel, L"focus");
}

void OzUIWindowOnUnfocus(struct OzUIWindow *window)
{
    OzUILabelSetText(window->titleLabel, L"unfocus");
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
    memset(window, 0, sizeof(struct OzUIWindow));
    window->id = reply.id;
    copyRect(&window->screenRect, &reply.screenRect);
    initRect(&window->clientRect, 0, 0, w, h);
    window->ops = &genericWindowOperation;
    INIT_LIST_HEAD(&window->widgetList);
    OzUIAppAddWindow(window);
    if (window->ops && window->ops->onCreate)
        window->ops->onCreate(window);

    struct Rect titleRect;
    wchar_t titleText[100];
    titleRect.x = titleRect.y = 0;
    titleRect.w = window->screenRect.w - 20;
    titleRect.h = 20;
    window->titleLabel = OzUICreateLabel(window, &titleRect);
    OzUILabelSetFontSize(window->titleLabel, 14);
    swprintf(titleText, 100, L"窗口：id=%p", window->id);
    OzUILabelSetText(window->titleLabel, titleText);

    struct Rect buttonRect;
    initRect(&buttonRect, rectRight(&titleRect), rectTop(&titleRect), 20, 20);
    window->closeButton = OzUICreateCloseButton(window, &buttonRect, 0);

    initRect(&window->clientRect, 0, 20, w, h - 20);

    return window;
}

int OzUIDestroyWindow(struct OzUIWindow *window)
{
    struct OzUIDestroyWindowRequest request;
    struct OzUIDestroyWindowReply reply;
    struct OzUIWidget *widget, *tmp;
    if (window->ops && window->ops->onDestroy)
        window->ops->onDestroy(window);
    listForEachEntrySafe(widget, tmp, &window->widgetList, link) {
        OzUIDestroyWidget(widget);
    }
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
    window->screenRect.x = x;
    window->screenRect.y = y;
    return reply.ret;
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

int OzUIWindowDrawLine(struct OzUIWindow *window, struct Rect *clipRect,
        struct Point *p1, struct Point *p2, struct LineStyle *lineStyle)
{
    struct OzUIWindowDrawLineRequest request;
    struct OzUIWindowDrawLineReply reply;
    request.type = OZUI_EVENT_DRAW_LINE;
    request.id = window->id;
    memcpy(&request.clipRect, clipRect, sizeof(struct Rect));
    memcpy(&request.p1, p1, sizeof(struct Point));
    memcpy(&request.p2, p2, sizeof(struct Point));
    memcpy(&request.lineStyle, lineStyle, sizeof(struct LineStyle));
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


