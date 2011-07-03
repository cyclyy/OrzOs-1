#include "syscall.h"

DEFN_SYSCALL_0(OzTestSyscall, 0);
DEFN_SYSCALL_1(OzPutChar, 1, char);
DEFN_SYSCALL_1(OzCreateServer, 2, s64int);
DEFN_SYSCALL_1(OzDestroyServer, 3, s64int);
DEFN_SYSCALL_1(OzConnect, 4, s64int);
DEFN_SYSCALL_1(OzDisconnect, 5, s64int);
DEFN_SYSCALL_5(OzSend, 6, s64int, u64int, void *, u64int, void *);
DEFN_SYSCALL_3(OzPost, 7, s64int, u64int, void *);
DEFN_SYSCALL_4(OzReceive, 8, s64int, u64int, void *, struct MessageInfo *);
DEFN_SYSCALL_3(OzReply, 9, s64int, u64int, void *);
DEFN_SYSCALL_2(OzNewTask, 10, char *, u64int);
DEFN_SYSCALL_1(OzExitTask, 11, s64int);
DEFN_SYSCALL_2(OzOpen, 12, char *, s64int);
DEFN_SYSCALL_1(OzClose, 13, s64int);
DEFN_SYSCALL_3(OzRead, 14, s64int, u64int, void *);
DEFN_SYSCALL_3(OzWrite, 15, s64int, u64int, void *);
DEFN_SYSCALL_3(OzReadDirectory, 16, s64int, u64int, void *);
DEFN_SYSCALL_1(OzAddHeapSize, 17, s64int);
DEFN_SYSCALL_4(OzIoControl, 18, u64int, s64int, u64int, void *);
DEFN_SYSCALL_3(OzSeek, 19, s64int, s64int, s64int);
