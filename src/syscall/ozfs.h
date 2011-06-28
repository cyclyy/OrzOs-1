#ifndef OZFS_H
#define OZFS_H

#include "sysdef.h"

s64int OzOpen(char *path, s64int flags);

s64int OzClose(s64int fd);

s64int OzRead(s64int fd, u64int offset, u64int size, char *buffer);

s64int OzWrite(s64int fd, u64int offset, u64int size, char *buffer);

s64int OzIoControl(s64int fd, s64int request, char *buffer, u64int size);

s64int OzReadDirectory(s64int fd, u64int bufSize, char *buffer);

#endif // OZFS_H
