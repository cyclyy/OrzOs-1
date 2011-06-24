#include "syscall.h"

DEFN_SYSCALL_0(TestSyscall, 0);
DEFN_SYSCALL_1(PutChar, 1, char);
DEFN_SYSCALL_1(CreateServer, 2, s64int);
DEFN_SYSCALL_1(DestroyServer, 3, s64int);
DEFN_SYSCALL_1(Connect, 4, s64int);
DEFN_SYSCALL_1(Disconnect, 5, s64int);
DEFN_SYSCALL_5(Send, 6, s64int, char *, u64int, char *, u64int);
DEFN_SYSCALL_3(Post, 7, s64int, char *, u64int);
DEFN_SYSCALL_3(Receive, 8, s64int, char *, u64int);
DEFN_SYSCALL_3(Reply, 9, s64int, char *, u64int);
DEFN_SYSCALL_2(NewTask, 10, char *, u64int);
DEFN_SYSCALL_1(ExitTask, 11, s64int);
DEFN_SYSCALL_2(Open, 12, char *, s64int);
DEFN_SYSCALL_1(Close, 13, s64int);
DEFN_SYSCALL_4(Read, 14, s64int, u64int, u64int, char *);
DEFN_SYSCALL_4(Write, 15, s64int, u64int, u64int, char *);
DEFN_SYSCALL_3(ReadDirectory, 16, s64int, u64int, char *);


