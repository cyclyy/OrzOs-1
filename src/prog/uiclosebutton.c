#include "uiclosebutton.h"
#include "uibutton.h"
#include "uiwindow.h"
#include "uiwidget.h"

static struct LineStyle whiteLS = {
    .color = {255,255,255},
    .lineWidth = 1,
};

static struct LineStyle blackLS = {
    .color = {0,0,0},
    .lineWidth = 1,
};

static struct FillStyle whiteFS = {
    .color = {255,255,255},
};

static struct FillStyle blackFS = {
    .color = {0,0,0},
};

static void onCloseButtonLeftClick(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent);
static void paintCloseButton(struct OzUIButton *button);

static struct OzUIButtonOperation closeButtonOps = {
    .paint = &paintCloseButton,
    .onMiceLeftClick = &onCloseButtonLeftClick,
};

static void onCloseButtonLeftClick(struct OzUIButton *button, struct OzUIMiceEvent *miceEvent)
{
    OzUIDestroyWindow(button->widget->window);
}

static void paintCloseButton(struct OzUIButton *button)
{
    struct Rect rect;
    struct LineStyle *ls;
    struct FillStyle *fs;
    struct Point p1, p2;

    initRect(&rect, 0, 0, button->widget->rect.w, button->widget->rect.h);
    switch (button->state) {
    case OZUI_BUTTON_STATE_READY:
        ls = &blackLS;
        fs = &whiteFS;
        break;
    case OZUI_BUTTON_STATE_MICE_HOVER:
        ls = &whiteLS;
        fs = &blackFS;
        break;
    case OZUI_BUTTON_STATE_MICE_DOWN:
        ls = &whiteLS;
        fs = &blackFS;
        break;
    }

    OzUIWidgetDrawRectangle(button->widget, &rect, ls, fs);

    p1.x = 0;
    p1.y = 0;
    p2.x = rect.w;
    p2.y = rect.h;
    OzUIWidgetDrawLine(button->widget, &p1, &p2, ls);

    p1.x = rect.w;
    p1.y = 0;
    p2.x = 0;
    p2.y = rect.h;
    OzUIWidgetDrawLine(button->widget, &p1, &p2, ls);
}

struct OzUIButton *OzUICreateCloseButton(struct OzUIWindow *window, struct Rect *rect, void *userData)
{
    return OzUICreateButton(window, rect, &closeButtonOps, userData);
}
