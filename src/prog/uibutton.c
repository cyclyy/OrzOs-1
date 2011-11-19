#include "uibutton.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static void buttonPaint(struct OzUIWidget *widget);
static void buttonMiceEnter(struct OzUIWidget *widget);
static void buttonMiceLeave(struct OzUIWidget *widget);
static void buttonMiceEvent(struct OzUIWidget *widget, struct OzUIMiceEvent *miceEvent);

static struct OzUIWidgetOperation buttonOps = {
    .onMiceEnter = &buttonMiceEnter,
    .onMiceLeave = &buttonMiceLeave,
    .onMiceEvent = &buttonMiceEvent,
    .paint = &buttonPaint,
};

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

static void buttonPaint(struct OzUIWidget *widget)
{
    struct Rect rect;
    struct OzUIButton *button;
    struct OzUITextLayoutConstraint tlc;
    struct LineStyle *ls;
    struct FillStyle *fs;

    button = (struct OzUIButton*)OzUIWidgetGetUserData(widget);
    if (button->ops && button->ops->paint) {
        return button->ops->paint(button);
    }

    initRect(&rect, 0, 0, widget->rect.w, widget->rect.h);
    copyRect(&tlc.rect, &rect);
    tlc.fontSize = 12;
    tlc.originX = tlc.originY = 0;
    tlc.flags = OZUI_TEXT_ALIGN_CENTER | OZUI_TEXT_ALIGN_VCENTER;
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

    OzUIWidgetDrawRectangle(widget, &rect, ls, fs);
    if (button && button->text)
        OzUIWidgetDrawText(widget, &tlc, button->text, ls, 0);
}

static void buttonMiceEnter(struct OzUIWidget *widget)
{
    OzUIButtonSetState((struct OzUIButton*)widget->d, OZUI_BUTTON_STATE_MICE_HOVER);
}

static void buttonMiceLeave(struct OzUIWidget *widget)
{
    OzUIButtonSetState((struct OzUIButton*)widget->d, OZUI_BUTTON_STATE_READY);
}

static void buttonMiceEvent(struct OzUIWidget *widget, struct OzUIMiceEvent *miceEvent)
{
    struct OzUIButton *button;

    button = (struct OzUIButton*)widget->d;
    if (!button->ops)
        return;
    if (button->ops->onMiceEvent)
        button->ops->onMiceEvent(button, miceEvent);
    if (miceEvent->type == OZUI_MICE_EVENT_DOWN) {
        if (miceEvent->button == OZUI_MICE_BUTTON_LEFT
                && button->ops->onMiceLeftClick) {
            button->ops->onMiceLeftClick(button, miceEvent);
        }
        if (miceEvent->button == OZUI_MICE_BUTTON_RIGHT
                && button->ops->onMiceRightClick) {
            button->ops->onMiceRightClick(button, miceEvent);
        }
    }
}

struct OzUIButton *OzUICreateButton(struct OzUIWindow *window, struct Rect *rect, struct OzUIButtonOperation *ops, void *userData)
{
    struct OzUIButton *button;

    button = (struct OzUIButton*)malloc(sizeof(struct OzUIButton));
    memset(button, 0, sizeof(struct OzUIButton));
    button->widget = OzUICreateWidget(window, OZUI_WIDGET_TYPE_BUTTON, rect, &buttonOps, button);
    button->state = OZUI_BUTTON_STATE_READY;
    button->ops = ops;
    button->d = userData;
    OzUIWidgetInvalidateAll(button->widget);
    return button;
}

void OzUIDestroyButton(struct OzUIButton *button)
{
    OzUIDestroyWidget(button->widget);
    free(button->text);
    free(button);
}

const wchar_t *OzUIButtonGetText(struct OzUIButton *button)
{
    return button->text;
}

void OzUIButtonSetText(struct OzUIButton *button, const wchar_t *text)
{
    free(button->text);
    button->text = wcsdup(text);
    OzUIWidgetInvalidateAll(button->widget);
}

int OzUIButtonGetState(struct OzUIButton *button)
{
    return button->state;
}

void OzUIButtonSetState(struct OzUIButton *button, int state)
{
    if (button->state != state) {
        button->state = state;
        OzUIWidgetInvalidateAll(button->widget);
    }
}


