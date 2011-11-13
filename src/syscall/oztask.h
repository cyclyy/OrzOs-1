#ifndef OZTASK_H
#define OZTASK_H

#include "sysdef.h"

s64int OzNewTask(char *path, u64int flags);

s64int OzExitTask(s64int status);

u64int OzAddHeapSize(s64int incr);

s64int OzGetPid();

#endif // OZTASK_H
