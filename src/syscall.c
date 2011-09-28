#include "syscall.h"

DEFN_SYSCALL_0(OzTestSyscall,   0);
DEFN_SYSCALL_1(OzPutChar,       1, char);
DEFN_SYSCALL_3(OzPost,          2, s64int, void *, u64int);
DEFN_SYSCALL_3(OzSend,          3, s64int, void *, u64int);
DEFN_SYSCALL_3(OzReceive,       4, struct MessageHeader *, void *, u64int);
DEFN_SYSCALL_0(OzGetPid,        5);
DEFN_SYSCALL_2(OzNewTask,       6, char *, u64int);
DEFN_SYSCALL_1(OzExitTask,      7, s64int);
DEFN_SYSCALL_2(OzOpen,          8, char *, s64int);
DEFN_SYSCALL_1(OzClose,         9, s64int);
DEFN_SYSCALL_3(OzRead,          10, s64int, u64int, void *);
DEFN_SYSCALL_3(OzWrite,         11, s64int, u64int, void *);
DEFN_SYSCALL_3(OzReadDirectory, 12, s64int, u64int, void *);
DEFN_SYSCALL_1(OzAddHeapSize,   13, s64int);
DEFN_SYSCALL_4(OzIoControl,     14, u64int, s64int, u64int, void *);
DEFN_SYSCALL_3(OzSeek,          15, s64int, s64int, s64int);
DEFN_SYSCALL_4(OzMap,           16, s64int, u64int, u64int, s64int);
DEFN_SYSCALL_3(OzReadAsync,     17, s64int, u64int, void *);
DEFN_SYSCALL_1(OzMilliAlarm, 	18, u64int);
DEFN_SYSCALL_1(OzMilliSleep, 	19, u64int);

