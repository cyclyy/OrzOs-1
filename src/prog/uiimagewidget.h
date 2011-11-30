#ifndef UIIMAGEWIDGET_H
#define UIIMAGEWIDGET_H

#include "uidef.h"

#define OZUI_WIDGET_TYPE_IMAGE_WIDGET  4

struct OzUIWindow;
struct OzUIWidget;

struct OzUIImageWidget
{
    struct OzUIWidget *widget;
    char path[MAX_PATH_LEN];
};

struct OzUIImageWidget *OzUICreateImageWidget(struct OzUIWindow *window, struct Rect *rect);

void OzUIDestroyImageWidget(struct OzUIImageWidget *imageWidget);

const char *OzUIImageWidgetGetPath(struct OzUIImageWidget *imageWidget);

void OzUIImageWidgetSetPath(struct OzUIImageWidget *imageWidget, const char *path);

#endif /* UIIMAGEWIDGET_H */
