#include "uikey.h"

static int keyLeftAlt = 0;
static int keyRightAlt = 0;
static int keyLeftShift = 0;
static int keyRightShift = 0;
static int keyLeftControl = 0;
static int keyRightControl = 0;

static int keyMask = OZUI_KEY_MASK_NULL;

int getKeyMask()
{
    return keyMask;
}

int updateKeyMask(struct KeyEvent *kev)
{
    int v;

    if (kev->type == KEY_EVENT_DOWN)
        v = 1;
    else
        v = 0;
    switch (kev->code) {
    case Key_LeftAlt:
        keyLeftAlt = v;
        break;
    case Key_RightAlt:
        keyRightAlt = v;
        break;
    case Key_LeftShift:
        keyLeftShift = v;
        break;
    case Key_RightShift:
        keyRightShift = v;
        break;
    case Key_LeftControl:
        keyLeftControl = v;
        break;
    case Key_RightControl:
        keyRightControl = v;
        break;
    default:
        ;
    }
    if (keyLeftAlt || keyRightAlt)
        keyMask |= OZUI_KEY_MASK_ALT;
    else
        keyMask &= ~OZUI_KEY_MASK_ALT;

    if (keyLeftShift || keyRightShift)
        keyMask |= OZUI_KEY_MASK_SHIFT;
    else
        keyMask &= ~OZUI_KEY_MASK_SHIFT;

    if (keyLeftControl || keyRightControl)
        keyMask |= OZUI_KEY_MASK_CONTROL;
    else
        keyMask &= ~OZUI_KEY_MASK_CONTROL;
    return keyMask;    
}

