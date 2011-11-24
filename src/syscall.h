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

struct MessageHeader
{
    int pid;
    int tstamp;
    unsigned long size;
};

DECL_SYSCALL_0(OzTestSyscall);
DECL_SYSCALL_1(OzPutChar, char);
DECL_SYSCALL_3(OzPost, s64int,  void *, u64int);
DECL_SYSCALL_3(OzSend, s64int, void *, u64int);
DECL_SYSCALL_3(OzReceive, struct MessageHeader *, void *, u64int);
DECL_SYSCALL_0(OzGetPid);
DECL_SYSCALL_2(OzNewTask, char *, u64int);
DECL_SYSCALL_1(OzExitTask, s64int);
DECL_SYSCALL_2(OzOpen, char *, s64int);
DECL_SYSCALL_1(OzClose, s64int);
DECL_SYSCALL_3(OzRead, s64int, u64int, void *);
DECL_SYSCALL_3(OzWrite, s64int, u64int, void *);
DECL_SYSCALL_3(OzReadDirectory, s64int, u64int, void *);
DECL_SYSCALL_1(OzAddHeapSize, s64int);
DECL_SYSCALL_4(OzIoControl, u64int, s64int, u64int, void *);
DECL_SYSCALL_3(OzSeek, s64int, s64int, s64int);
DECL_SYSCALL_4(OzMap, s64int, u64int, u64int, s64int);
DECL_SYSCALL_3(OzReadAsync, s64int, u64int, void *);
DECL_SYSCALL_1(OzMilliAlarm, u64int);
DECL_SYSCALL_1(OzMilliSleep, u64int);
DECL_SYSCALL_5(OzSendReceive, struct MessageHeader *, void *, u64int, void *, u64int);
DECL_SYSCALL_3(OzReply, s64int, void *, u64int);
DECL_SYSCALL_0(OzGetTicks);

#endif /* SYSCALL_H */
