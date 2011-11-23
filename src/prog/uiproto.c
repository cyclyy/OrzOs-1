#include "uiproto.h"
#include "uiwindow.h"
#include "uiwidget.h"
#include "uiapp.h"
#include <os/event.h>
#include <os/list.h>
#include <os/mice.h>
#include <os/sysdef.h>
#include <os/syscall.h>
#include <string.h>
#include <stdlib.h>

int OzUISendReceive(void *request, void *reply)
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
        case OZUI_EVENT_DRAW_LINE:
            return OzSendReceive(&hdr, request, sizeof(struct OzUIWindowDrawLineRequest), reply, sizeof(struct OzUIWindowDrawLineReply));
            break;
        case OZUI_EVENT_DRAW_TEXT:
            return OzSendReceive(&hdr, request, SIZE_OZUI_WINDOW_DRAW_TEXT_REQUEST(request), reply, SIZE_OZUI_WINDOW_DRAW_TEXT_REPLY_FOR_TEXT(((struct OzUIWindowDrawTextRequest*)request)->text));
            break;
    };
    return 0;
}

int OzUISend(void *request)
{
    switch (GET_EVENT_TYPE(request)) {
        case OZUI_EVENT_NEXT_EVENT:
            return OzSend(UISERVER_PID, request, sizeof(struct OzUINextEventRequest));
            break;
    };
    return 0;
}

int OzUIDispatchEvent(void *buf)
{
    struct OzUIMiceEventNotify *uiMiceEventNotify;
    struct OzUIFocusEventNotify *uiFocusEventNotify;
    struct OzUIWindow *window;
    int isUIEvent = 1;
    switch GET_EVENT_TYPE(buf) {
        case OZUI_EVENT_MICE:
            uiMiceEventNotify = (struct OzUIMiceEventNotify*)buf;
            window = OzUIGetWindowById(uiMiceEventNotify->id);
            if (window && window->ops && window->ops->onMiceEvent)
                window->ops->onMiceEvent(window, &uiMiceEventNotify->miceEvent);
            break;
        case OZUI_EVENT_FOCUS:
            uiFocusEventNotify = (struct OzUIFocusEventNotify*)buf;
            window = OzUIGetWindowById(uiFocusEventNotify->id);
            if (window && window->ops && window->ops->onFocus)
                window->ops->onFocus(window);
            break;
        case OZUI_EVENT_UNFOCUS:
            uiFocusEventNotify = (struct OzUIFocusEventNotify*)buf;
            window = OzUIGetWindowById(uiFocusEventNotify->id);
            if (window && window->ops && window->ops->onUnfocus)
                window->ops->onUnfocus(window);
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

// vim: sw=4 sts=4 et tw=100
