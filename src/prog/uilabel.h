#ifndef UILABEL_H
#define UILABEL_H

#include "uiproto.h"

#define OZUI_WIDGET_TYPE_LABEL  1
#define OZUI_LABEL_DEFAULT_FONT_SIZE 12

struct OzUILabel
{
    struct OzUIWidget *widget;
    int fontSize;
    wchar_t *text;
};

struct OzUILabel *OzUICreateLabel(struct OzUIWindow *window, struct Rect *rect);

void OzUIDestroyLabel(struct OzUILabel *label);

const wchar_t *OzUILabelGetText(struct OzUILabel *label);

void OzUILabelSetText(struct OzUILabel *label, const wchar_t *text);

int OzUILabelGetFontSize(struct OzUILabel *label);

void OzUILabelSetFontSize(struct OzUILabel *label, int fontSize);

#endif /* UILABEL_H */
