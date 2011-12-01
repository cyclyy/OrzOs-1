#include "uikey.h"
#include "utf8.h"
#include <os/key.h>

wchar_t OzUIKeyEventTranslate(struct OzUIKeyEvent *kev)
{
    char ch;
    wchar_t wch;
    if (!KEY_CODE_IS_PRINT(kev->code))
        return (wchar_t)0;
    ch = (char)kev->code;
    wch = 0;
    if (KEY_CODE_IS_ALPHA(kev->code)) {
        if (kev->mask == OZUI_KEY_MASK_NULL) {
            ch |= 32;
            utowc(&wch, &ch, 1);
        //} else if (kev->mask == OZUI_KEY_MASK_SHIFT) {
        } else if (kev->mask) {
            utowc(&wch, &ch, 1);
        }
    } else if (kev->mask == OZUI_KEY_MASK_NULL) {
        utowc(&wch, &ch, 1);
    } else if (kev->mask == OZUI_KEY_MASK_SHIFT) {
        switch (kev->code) {
        case Key_Semicolon:
            ch = Key_Colon;
            break;
        case Key_Comma:
            ch = Key_Less;
            break;
        case Key_Period:
            ch = Key_Greater;
            break;
        case Key_Slash:
            ch = Key_Question;
            break;
        default:
            return 0;
        }
        utowc(&wch, &ch, 1);
    }
    return wch;
}
