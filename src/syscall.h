#ifndef SYSCALL_H
#define SYSCALL_H 

#include "common.h"

#define DECL_SYSCALL_0(fn) u32int syscall_##fn(); 
#define DECL_SYSCALL_1(fn,p1) u32int syscall_##fn(p1); 
#define DECL_SYSCALL_2(fn,p1,p2) u32int syscall_##fn(p1,p2); 
#define DECL_SYSCALL_3(fn,p1,p2,p3) u32int syscall_##fn(p1,p2,p3); 
#define DECL_SYSCALL_4(fn,p1,p2,p3,p4) u32int syscall_##fn(p1,p2,p3,p4); 
#define DECL_SYSCALL_5(fn,p1,p2,p3,p4,p5) u32int syscall_##fn(p1,p2,p3,p4,p5); 

#define DEFN_SYSCALL_0(fn,num) \
    u32int syscall_##fn()   \
    {                       \
        u32int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num)); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_1(fn,num,P1) \
    u32int syscall_##fn(P1 p1)   \
    {                       \
        u32int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u32int)p1) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_2(fn,num,P1,P2) \
    u32int syscall_##fn(P1 p1, P2 p2)   \
    {                       \
        u32int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u32int)p1), \
                "c"((u32int)p2) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_3(fn,num,P1,P2,P3) \
    u32int syscall_##fn(P1 p1, P2 p2, P3 p3)   \
    {                       \
        u32int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u32int)p1), \
                "c"((u32int)p2), "d"((u32int)p3) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_4(fn,num,P1,P2,P3,P4) \
    u32int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4)   \
    {                       \
        u32int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u32int)p1), \
                "c"((u32int)p2), "d"((u32int)p3), "S"((u32int)p4) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_5(fn,num,P1,P2,P3,P4,P5) \
    u32int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)   \
    {                       \
        u32int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u32int)p1), \
                "c"((u32int)p2), "d"((u32int)p3), "S"((u32int)p4), "D"((u32int)p5) ); \
        return ret;         \
    }                       

DECL_SYSCALL_0(fork);
DECL_SYSCALL_3(execve, char*, char**, char**);
DECL_SYSCALL_1(exit, s32int);
DECL_SYSCALL_2(open, char*, u32int);
DECL_SYSCALL_1(close, s32int);
DECL_SYSCALL_3(read, s32int, void*, u32int);
DECL_SYSCALL_3(write, s32int, void*, u32int);
DECL_SYSCALL_3(lseek, s32int, s32int, u32int);
DECL_SYSCALL_5(mmap, u32int, u32int, s32int, s32int, u32int);
DECL_SYSCALL_2(unmap, u32int, u32int);
DECL_SYSCALL_1(msleep, u32int);
DECL_SYSCALL_2(mkdir, char*,u32int);
DECL_SYSCALL_3(mknod, char*,u32int,u32int);
DECL_SYSCALL_2(create, char*,u32int);
DECL_SYSCALL_1(rmdir, char*);
DECL_SYSCALL_1(rm, char*);
DECL_SYSCALL_3(getdents,u32int,u8int*,u32int);
DECL_SYSCALL_2(getcwd, char*,u32int);
DECL_SYSCALL_1(chdir, char*);
DECL_SYSCALL_1(putch, char);

#endif /* SYSCALL_H */
