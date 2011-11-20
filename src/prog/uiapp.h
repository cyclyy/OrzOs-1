#ifndef UIAPP_H
#define UIAPP_H

#include "uiproto.h"
#include "libc/list.h"

struct OzUIApp
{
    wchar_t name[100];
    struct ListHead windowList;
};

extern struct OzUIApp *app;

void OzUIAppInit(int argc, char *argv[]);

void OzUIAppQuit();

void OzUIAppAddWindow(struct OzUIWindow *window);

void OzUIAppRemoveWindow(struct OzUIWindow *window);

#endif /* UIAPP_H */
