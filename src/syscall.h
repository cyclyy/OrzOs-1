#ifndef SYSCALL_H
#define SYSCALL_H

#include "sysdef.h"

#define DECL_SYSCALL_0(fn) u64int fn(); 
#define DECL_SYSCALL_1(fn,p1) u64int fn(p1); 
#define DECL_SYSCALL_2(fn,p1,p2) u64int fn(p1,p2); 
#define DECL_SYSCALL_3(fn,p1,p2,p3) u64int fn(p1,p2,p3); 
#define DECL_SYSCALL_4(fn,p1,p2,p3,p4) u64int fn(p1,p2,p3,p4); 
#define DECL_SYSCALL_5(fn,p1,p2,p3,p4,p5) u64int fn(p1,p2,p3,p4,p5); 

#define DEFN_SYSCALL_0(fn,num) \
    u64int fn()   \
    {                       \
        u64int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num)); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_1(fn,num,P1) \
    u64int fn(P1 p1)   \
    {                       \
        u64int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u64int)p1) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_2(fn,num,P1,P2) \
    u64int fn(P1 p1, P2 p2)   \
    {                       \
        u64int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u64int)p1), \
                "c"((u64int)p2) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_3(fn,num,P1,P2,P3) \
    u64int fn(P1 p1, P2 p2, P3 p3)   \
    {                       \
        u64int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u64int)p1), \
                "c"((u64int)p2), "d"((u64int)p3) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_4(fn,num,P1,P2,P3,P4) \
    u64int fn(P1 p1, P2 p2, P3 p3, P4 p4)   \
    {                       \
        u64int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u64int)p1), \
                "c"((u64int)p2), "d"((u64int)p3), "S"((u64int)p4) ); \
        return ret;         \
    }                       

#define DEFN_SYSCALL_5(fn,num,P1,P2,P3,P4,P5) \
    u64int fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)   \
    {                       \
        u64int ret;         \
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"((u64int)p1), \
                "c"((u64int)p2), "d"((u64int)p3), "S"((u64int)p4), "D"((u64int)p5) ); \
        return ret;         \
    }                       

DECL_SYSCALL_0(TestSyscall);
DECL_SYSCALL_1(PutChar, char);
DECL_SYSCALL_1(CreateServer, s64int);
DECL_SYSCALL_1(DestroyServer, s64int);
DECL_SYSCALL_1(Connect, s64int);
DECL_SYSCALL_1(Disconnect, s64int);
DECL_SYSCALL_5(Send, s64int, char *, u64int, char *, u64int);
DECL_SYSCALL_3(Post, s64int, char *, u64int);
DECL_SYSCALL_3(Receive, s64int, char *, u64int);
DECL_SYSCALL_3(Reply, s64int, char *, u64int);
DECL_SYSCALL_2(NewTask, char *, u64int);
DECL_SYSCALL_1(ExitTask, s64int);
DECL_SYSCALL_2(Open, char *, s64int);
DECL_SYSCALL_1(Close, s64int);
DECL_SYSCALL_4(Read, s64int, u64int, u64int, char *);
DECL_SYSCALL_4(Write, s64int, u64int, u64int, char *);
DECL_SYSCALL_3(ReadDirectory, s64int, u64int, char *);

#endif /* SYSCALL_H */
