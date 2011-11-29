#include "uitextlayout.h"
#include "rect.h"

void OzUITextLayoutTransform(struct OzUITextLayout *tl, int deltaX, int deltaY)
{
    int i;
    if ((deltaX == 0) && (deltaY == 0))
        return;
    translateRect(&tl->rect, deltaX, deltaY);
    for (i=0; i<tl->chars; i++) {
        translateRect(&tl->charLayout[i].rect, deltaX, deltaY);
    }
}
