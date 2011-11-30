#include "uiapp.h"
#include "uiwindow.h"
#include <os/list.h>
#include <stdlib.h>

static struct OzUIApp _app = {
    .name = L"",
    .windowList = LIST_HEAD_INIT(_app.windowList),
};

struct OzUIApp *app = &_app;

void OzUIAppInit(int argc, char *argv[])
{
}

void OzUIAppQuit()
{
    struct OzUIWindow *window, *tmp;
    listForEachEntrySafe(window, tmp, &app->windowList, link) {
        OzUIDestroyWindow(window);
    }
}


void OzUIAppAddWindow(struct OzUIWindow *window)
{
    listAddTail(&window->link, &app->windowList); 
}

void OzUIAppRemoveWindow(struct OzUIWindow *window)
{
    listDel(&window->link);
}

