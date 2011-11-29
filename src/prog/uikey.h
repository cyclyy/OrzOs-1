#ifndef UIKEY_H
#define UIKEY_H

#include <os/key.h>
#include <wchar.h>

#define OZUI_KEY_EVENT_UP       KEY_EVENT_UP
#define OZUI_KEY_EVENT_DOWN     KEY_EVENT_DOWN

#define OZUI_KEY_MASK_NULL      0
#define OZUI_KEY_MASK_ALT       1
#define OZUI_KEY_MASK_SHIFT     2
#define OZUI_KEY_MASK_CONTROL   4

struct OzUIKeyEvent
{
    int type;
    int code;
    int mask;
}__attribute__((packed));

struct OzUIKeyEventNotify
{
    int type;
    unsigned long id;
    struct OzUIKeyEvent keyEvent;
}__attribute__((packed));

wchar_t OzUIKeyEventTranslate(struct OzUIKeyEvent *kev);

#endif /* UIKEY_H */
