#include "uiwindow.h"
#include "uiapp.h"
#include <stdlib.h>
#include <string.h>

void OzUIWindowOnMiceEvent(struct OzUIWindow *window, struct OzUIMiceEvent *miceEvent);

struct OzUIWindowOperation genericWindowOperation = {
    .onMiceEvent = &OzUIWindowOnMiceEvent,
};

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


