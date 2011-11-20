#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "uiproto.h"
#include "uiwindow.h"
#include <os/mice.h>

#define OZUI_WIDGET_TYPE_BUTTON         2

#define OZUI_BUTTON_STATE_READY         1
#define OZUI_BUTTON_STATE_MICE_HOVER    2
#define OZUI_BUTTON_STATE_MICE_DOWN     3

struct OzUIButtonOperation;
struct OzUIButton
{
    struct OzUIWidget *widget;
    wchar_t *text;
    int state;
    struct OzUIButtonOperation *ops;
    void *d;
};

struct OzUIButtonOperation
{
    void (*onCreate)(struct OzUIButton *button);
    void (*onDestroy)(struct OzUIButton *button);
    void (*onMiceEnter)(struct OzUIButton *button);
    void (*onMiceLeave)(struct OzUIButton *button);
    void (*onMiceLeftClick)(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent);
    void (*onMiceRightClick)(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent);
    void (*onMiceEvent)(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent);
    void (*paint)(struct OzUIButton *button);
};

struct OzUIButton *OzUICreateButton(struct OzUIWindow *window, struct Rect *rect, struct OzUIButtonOperation *ops, void *userData);

void OzUIDestroyButton(struct OzUIButton *button);

const wchar_t *OzUIButtonGetText(struct OzUIButton *button);

void OzUIButtonSetText(struct OzUIButton *button, const wchar_t *text);

int OzUIButtonGetState(struct OzUIButton *button);

void OzUIButtonSetState(struct OzUIButton *button, int state);

#endif /* UIBUTTON_H */
