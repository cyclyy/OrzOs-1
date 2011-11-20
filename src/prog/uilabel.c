#include "uilabel.h"
#include "uiwindow.h"
#include "uiwidget.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static void labelPaint(struct OzUIWidget *widget);

static struct OzUIWidgetOperation labelOps = {
    .paint = &labelPaint,
};

static struct LineStyle blackLS = {
    .color = {0,0,0},
    .lineWidth = 0,
};

static struct FillStyle whiteFS = {
    .color = {255,255,255},
};

static void labelPaint(struct OzUIWidget *widget)
{
    struct Rect rect;
    struct OzUILabel *label;
    struct OzUITextLayoutConstraint tlc;

    label = (struct OzUILabel*)widget->d;
    initRect(&rect, 0, 0, widget->rect.w, widget->rect.h);
    copyRect(&tlc.rect, &rect);
    tlc.fontSize = label->fontSize;
    tlc.originX = tlc.originY = 0;
    tlc.flags = OZUI_TEXT_ALIGN_VCENTER;
    OzUIWidgetDrawRectangle(widget, &rect, &blackLS, &whiteFS);
    if (label && label->text)
        OzUIWidgetDrawText(widget, &tlc, label->text, &blackLS, 0);
}

struct OzUILabel *OzUICreateLabel(struct OzUIWindow *window, struct Rect *rect)
{
    struct OzUILabel *label;

    label = (struct OzUILabel*)malloc(sizeof(struct OzUILabel));
    memset(label, 0, sizeof(struct OzUILabel));
    label->widget = OzUICreateWidget(window, OZUI_WIDGET_TYPE_LABEL, rect, &labelOps, label);
    label->fontSize = OZUI_LABEL_DEFAULT_FONT_SIZE;
    OzUIWidgetInvalidateAll(label->widget);
    return label;
}

void OzUIDestroyLabel(struct OzUILabel *label)
{
    OzUIDestroyWidget(label->widget);
    free(label->text);
    free(label);
}

const wchar_t *OzUILabelGetText(struct OzUILabel *label)
{
    return label->text;
}

void OzUILabelSetText(struct OzUILabel *label, const wchar_t *text)
{
    free(label->text);
    label->text = wcsdup(text);
    OzUIWidgetInvalidateAll(label->widget);
}

int OzUILabelGetFontSize(struct OzUILabel *label)
{
    return label->fontSize;
}

void OzUILabelSetFontSize(struct OzUILabel *label, int fontSize)
{
    label->fontSize = fontSize;
    OzUIWidgetInvalidateAll(label->widget);
}

