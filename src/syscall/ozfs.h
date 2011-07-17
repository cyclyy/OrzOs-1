#ifndef OZFS_H
#define OZFS_H

#include "sysdef.h"

s64int OzOpen(char *path, s64int flags);

s64int OzClose(s64int fd);

s64int OzRead(s64int fd, u64int size, char *buffer);

s64int OzWrite(s64int fd, u64int size, char *buffer);

s64int OzIoControl(s64int fd, s64int request, u64int size, char *buffer);

s64int OzReadDirectory(s64int fd, u64int bufSize, char *buffer);

s64int OzSeek(s64int fd, s64int offset, s64int pos);

s64int OzMap(s64int fd, u64int addr, u64int size, s64int flags);

#endif // OZFS_H
