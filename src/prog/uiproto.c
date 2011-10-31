#include "uiproto.h"
#include "uidef.h"
#include "sysdef.h"
#include "syscall.h"
#include "event.h"
#include <string.h>
#include <stdlib.h>

int OzUISendReceive(void *request, void *reply)
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
    };
    return 0;
}

unsigned long OzUICreateWindow(int w, int h, int flags)
{
    struct OzUICreateWindowRequest request;
    struct OzUICreateWindowReply reply;
    request.type = UI_EVENT_CREATE_WINDOW;
    request.width = w;
    request.height = h;
    request.flags = flags;
    OzUISendReceive(&request, &reply);
    return reply.id;
}

int OzUIDestroyWindow(unsigned long id)
{
    struct OzUIDestroyWindowRequest request;
    struct OzUIDestroyWindowReply reply;
    request.type = UI_EVENT_DESTROY_WINDOW;
    request.id = id;
    OzUISendReceive(&request, &reply);
    return reply.ret;
}

int OzUIMoveWindow(unsigned long id, int x, int y)
{
    struct OzUIMoveWindowRequest request;
    struct OzUIMoveWindowReply reply;
    request.type = UI_EVENT_MOVE_WINDOW;
    request.id = id;
    request.x = x;
    request.y = y;
    OzUISendReceive(&request, &reply);
    return reply.ret;
}

// vim: sw=4 sts=4 et tw=100
