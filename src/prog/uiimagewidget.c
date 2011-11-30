#include "uiimagewidget.h"
#include "uiwindow.h"
#include "uiwidget.h"
#include <stdlib.h>
#include <string.h>


static void imageWidgetPaint(struct OzUIWidget *widget);

static struct OzUIWidgetOperation imageWidgetOps = {
    .paint = &imageWidgetPaint,
};

static void imageWidgetPaint(struct OzUIWidget *widget)
{
    struct Rect rect;
    struct OzUIImageWidget *imageWidget;

    imageWidget = (struct OzUIImageWidget*)widget->d;
    initRect(&rect, 0, 0, widget->rect.w, widget->rect.h);
    OzUIWidgetDrawImageFile(widget, 0, 0, imageWidget->path);
}

struct OzUIImageWidget *OzUICreateImageWidget(struct OzUIWindow *window, struct Rect *rect)
{
    struct OzUIImageWidget *imageWidget;

    imageWidget = (struct OzUIImageWidget*)malloc(sizeof(struct OzUIImageWidget));
    memset(imageWidget, 0, sizeof(struct OzUIImageWidget));
    imageWidget->widget = OzUICreateWidget(window, OZUI_WIDGET_TYPE_IMAGE_WIDGET, 0, rect, &imageWidgetOps, imageWidget);
    OzUIWidgetInvalidateAll(imageWidget->widget);
    return imageWidget;
}

void OzUIDestroyImageWidget(struct OzUIImageWidget *imageWidget)
{
    OzUIDestroyWidget(imageWidget->widget);
    free(imageWidget);
}

const char *OzUIImageWidgetGetPath(struct OzUIImageWidget *imageWidget)
{
    return imageWidget->path;
}

void OzUIImageWidgetSetPath(struct OzUIImageWidget *imageWidget, const char *path)
{
    strcpy(imageWidget->path, path);
    OzUIWidgetInvalidateAll(imageWidget->widget);
}

