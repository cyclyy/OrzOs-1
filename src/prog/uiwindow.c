#include "uiwindow.h"
#include "uiproto.h"
#include "uiwidget.h"
#include "uiapp.h"
#include "uilabel.h"
#include "uiclosebutton.h"
#include "uitextlayout.h"
#include <stdlib.h>
#include <string.h>

void OzUIWindowOnKeyEvent(struct OzUIWindow *window, struct OzUIKeyEvent *keyEvent);
void OzUIWindowOnMiceEvent(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent);
void OzUIWindowOnFocus(struct OzUIWindow *window);
void OzUIWindowOnUnfocus(struct OzUIWindow *window);

struct OzUIWindowOperation genericWindowOperation = {
    .onKeyEvent = &OzUIWindowOnKeyEvent,
    .onMiceEvent = &OzUIWindowOnMiceEvent,
    .onFocus = &OzUIWindowOnFocus,
    .onUnfocus = &OzUIWindowOnUnfocus,
};

void OzUIWindowOnKeyEvent(struct OzUIWindow *window, struct OzUIKeyEvent *keyEvent)
{
    if (!window->focusWidget)
        return;
    if (window->focusWidget->ops && window->focusWidget->ops->onKeyEvent)
        window->focusWidget->ops->onKeyEvent(window->focusWidget, keyEvent);
}

static struct OzUIWidget *widgetUnderMice(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent, struct OzUIMiceEvent *localMiceEvent)
{
    struct OzUIWidget *widget;
    struct OzUIMiceEvent ev;
    memcpy(&ev, miceEvent, sizeof(struct MiceEvent));
    ev.x -= window->screenRect.x;
    ev.y -= window->screenRect.y;
    listForEachEntry(widget, &window->widgetList, link) {
        if (insideRect(&widget->rect, ev.x, ev.y)) {
            if (localMiceEvent) {
                memcpy(localMiceEvent, &ev, sizeof(struct MiceEvent));
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
    static int lastX, lastY;
    if (window->inDrag) {
        switch (miceEvent->type) {
        case OZUI_MICE_EVENT_DOWN:
        case OZUI_MICE_EVENT_MOVE:
            if (miceEvent->button == OZUI_MICE_BUTTON_LEFT)
                OzUIMoveWindow(window, miceEvent->x - lastX, miceEvent->y - lastY);
            else {
                window->inDrag = 0;
            }
            break;
        case OZUI_MICE_EVENT_UP:
            OzUIMoveWindow(window, miceEvent->x - lastX, miceEvent->y - lastY);
            window->inDrag = 0;
            break;
        }
        return;
    }
    widget = widgetUnderMice(window, miceEvent, &localMiceEvent);
    if (widget) {
        // handle keyboard focus event
        if ((miceEvent->type == OZUI_MICE_EVENT_DOWN) && (widget->flags & OZUI_WIDGET_FLAG_FOCUSABLE)) {
            if (window->focusWidget != widget) {
                if (window->focusWidget) {
                    if (window->focusWidget->ops && window->focusWidget->ops->onUnfocus) {
                        window->focusWidget->ops->onUnfocus(window->focusWidget);
                    }
                }
                window->focusWidget = widget;
                if (widget) {
                    if (widget->ops && widget->ops->onFocus) {
                        widget->ops->onFocus(widget);
                    } }
            }
        }
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
        localMiceEvent.x = miceEvent->x - window->screenRect.x - widget->rect.x;
        localMiceEvent.y = miceEvent->y - window->screenRect.y - widget->rect.y;
        if (widget->ops && widget->ops->onMiceEvent)
            widget->ops->onMiceEvent(widget, &localMiceEvent);
    } else {
        if (window->miceWidget) {
            localMiceEvent.type = OZUI_EVENT_MICE_LEAVE;
            if (window->miceWidget->ops && window->miceWidget->ops->onMiceLeave)
                window->miceWidget->ops->onMiceLeave(window->miceWidget);
        }
        // handle user drag window
        if (!window->inDrag) {
            if ((miceEvent->type == OZUI_MICE_EVENT_DOWN) && (miceEvent->button == OZUI_MICE_BUTTON_LEFT)) {
                window->inDrag = 1;
                lastX = miceEvent->x - window->screenRect.x;
                lastY = miceEvent->y - window->screenRect.y;
            }
        }
    }
    window->miceWidget = widget;
}

void OzUIWindowOnFocus(struct OzUIWindow *window)
{
//    OzUILabelSetText(window->titleLabel, L"focus");
}

void OzUIWindowOnUnfocus(struct OzUIWindow *window)
{
//    OzUILabelSetText(window->titleLabel, L"unfocus");
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

int OzUIWindowQueryTextLayout(struct OzUIWindow *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, const wchar_t *text, struct OzUITextLayout *layout)
{
    struct OzUIWindowQueryTextLayoutRequest *request;
    struct OzUIWindowQueryTextLayoutReply *reply;
    int i, j, ret;
    i = SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REQUEST_FOR_TEXT(text);
    j = SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REPLY_FOR_TEXT(text);
    request = (struct OzUIWindowQueryTextLayoutRequest*)malloc(SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REQUEST_FOR_TEXT(text));
    reply = (struct OzUIWindowQueryTextLayoutReply*)malloc(SIZE_OZUI_WINDOW_QUERY_TEXT_LAYOUT_REPLY_FOR_TEXT(text));
    request->type = OZUI_EVENT_QUERY_TEXT_LAYOUT;
    request->id = window->id;
    memcpy(&request->clipRect, clipRect, sizeof(struct Rect));
    memcpy(&request->tlc, tlc, sizeof(struct OzUITextLayoutConstraint));
    wcscpy(request->text, text);
    OzUISendReceive(request, reply);
    ret = reply->ret;
    i = SIZE_OZUI_TEXT_LAYOUT(&reply->layout);
    if (layout)
        memcpy(layout, &reply->layout, SIZE_OZUI_TEXT_LAYOUT(&reply->layout));
    free(request);
    free(reply);
    return ret;
}

int OzUIWindowDrawTextLayout(struct OzUIWindow *window, struct Rect *clipRect,
        struct OzUITextLayoutConstraint *tlc, struct LineStyle *lineStyle, struct OzUITextLayout *layout)
{
    struct OzUIWindowDrawTextLayoutRequest *request;
    struct OzUIWindowDrawTextLayoutReply reply;
    int n;
    n = SIZE_OZUI_WINDOW_DRAW_TEXT_LAYOUT_REQUEST_FOR_CHARS(layout->chars);
    request = (struct OzUIWindowDrawTextLayoutRequest*)malloc(n);
    request->type = OZUI_EVENT_DRAW_TEXT_LAYOUT;
    request->id = window->id;
    memcpy(&request->clipRect, clipRect, sizeof(struct Rect));
    memcpy(&request->tlc, tlc, sizeof(struct OzUITextLayoutConstraint));
    memcpy(&request->lineStyle, lineStyle, sizeof(struct LineStyle));
    memcpy(&request->layout, layout, SIZE_OZUI_TEXT_LAYOUT(layout));
    OzUISendReceive(request, &reply);
    free(request);
    return reply.ret;
}

int OzUIWindowDrawImageFile(struct OzUIWindow *window, struct Rect *clipRect,
        int x, int y, const char *path)
{
    struct OzUIWindowDrawImageFileRequest request;
    struct OzUIWindowDrawImageFileReply reply;
    memset(&request, 0, sizeof(struct OzUIWindowDrawImageFileRequest));
    request.type = OZUI_EVENT_DRAW_IMAGE_FILE;
    request.id = window->id;
    copyRect(&request.clipRect, clipRect);
    request.x = x;
    request.y = y;
    strncpy(request.path, path, MAX_PATH_LEN-1);
    OzUISendReceive(&request, &reply);
    return reply.ret;
}

