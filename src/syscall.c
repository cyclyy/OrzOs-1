#include "syscall.h"

DEFN_SYSCALL_0(TestSyscall, 0);
DEFN_SYSCALL_1(PutChar, 1, char);
DEFN_SYSCALL_1(CreateServer, 2, s64int);
DEFN_SYSCALL_1(DestroyServer, 3, s64int);
DEFN_SYSCALL_1(Connect, 4, s64int);
DEFN_SYSCALL_1(Disconnect, 5, s64int);
DEFN_SYSCALL_5(Send, 6, s64int, char *, u64int, char *, u64int);
DEFN_SYSCALL_3(Receive, 7, s64int, char *, u64int);
DEFN_SYSCALL_3(Reply, 8, s64int, char *, u64int);
DEFN_SYSCALL_2(NewTask, 9, char *, u64int);


