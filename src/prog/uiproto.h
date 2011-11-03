#ifndef UIPROTO_H
#define UIPROTO_H

#include "sysdef.h"
#include "uidef.h"

struct OzUIWindow *OzUICreateWindow(int w, int h, int flags);

int OzUIDestroyWindow(struct OzUIWindow *window);

int OzUIMoveWindow(struct OzUIWindow *window, int x, int y);

int OzUIReadEvent(struct OzUIWindow *window);

int OzUIDispatchEvent(void *buf);

void OzUINextEvent();

struct OzUIWindow *OzUIGetWindowById(unsigned long id);


#endif // UIPROTO_H
