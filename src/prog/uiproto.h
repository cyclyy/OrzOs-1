#ifndef UIPROTO_H
#define UIPROTO_H

#include <os/sysdef.h>

void OzUINextEvent();

int OzUIDispatchEvent(void *buf);

int OzUISendReceive(void *request, void *reply);

int OzUISend(void *request);

// widget related functions
#endif // UIPROTO_H
