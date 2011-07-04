#ifndef UIPROTO_H
#define UIPROTO_H

#include "sysdef.h"

s64int uiInitApp(const char *name, u64int flags);

s64int uiQuitApp(s64int id);

s64int uiConsoleRead(s64int id, u64int size, void *buf);

s64int uiConsoleWrite(s64int id, u64int size, void *buf);

#endif // UIPROTO_H
