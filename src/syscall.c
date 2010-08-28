#include "syscall.h"

DEFN_SYSCALL_0(fork, 0);
DEFN_SYSCALL_3(execve, 1, char*, char**, char**);
DEFN_SYSCALL_1(exit, 2, s32int);
DEFN_SYSCALL_2(open, 3, char*, u32int);
DEFN_SYSCALL_1(close, 4, s32int);
DEFN_SYSCALL_3(read, 5, s32int, void*, u32int);
DEFN_SYSCALL_3(write, 6, s32int, void*, u32int);
DEFN_SYSCALL_3(lseek, 7, s32int, s32int, u32int);
DEFN_SYSCALL_5(mmap, 8, u32int, u32int, s32int, s32int, u32int);
DEFN_SYSCALL_2(unmap, 9, u32int, u32int);
DEFN_SYSCALL_1(msleep, 10, u32int);
DEFN_SYSCALL_2(mkdir, 11,char*,u32int);
DEFN_SYSCALL_3(mknod, 12,char*,u32int,u32int);
DEFN_SYSCALL_2(create, 13,char*,u32int);
DEFN_SYSCALL_1(rmdir, 14,char*);
DEFN_SYSCALL_1(rm, 15,char*);
DEFN_SYSCALL_3(getdents,16,u32int,u8int*,u32int);
DEFN_SYSCALL_2(getcwd, 17,char*,u32int);
DEFN_SYSCALL_1(chdir, 18,char*);
DEFN_SYSCALL_1(putch, 19, char);


