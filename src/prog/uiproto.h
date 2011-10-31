#ifndef UIPROTO_H
#define UIPROTO_H

#include "sysdef.h"

int OzUISendReceive(void *request, void *reply);

unsigned long OzUICreateWindow(int w, int h, int flags);

int OzUIDestroyWindow(unsigned long id);

int OzUIMoveWindow(unsigned long id, int x, int y);

#endif // UIPROTO_H
