#include "ksyscall.h"
#include "interrupt.h"
#include "util.h"
#include "task.h"
#include "vfs.h"
#include "debugcon.h"
#include "syscall/ozipc.h"
#include "syscall/oztask.h"
#include "syscall/ozfs.h"

void syscallTest();

#define N_SYSCALLS 21

void *syscalls[N_SYSCALLS] = {
    &syscallTest,
    &dbgPutch,
    OzCreateServer,
    OzDestroyServer,
    OzConnect,
    OzDisconnect,
    OzSend,
    OzPost,
    OzReceive,
    OzReply,
    OzNewTask,
    OzExitTask,
    OzOpen,
    OzClose,
    OzRead,
    OzWrite,
    OzReadDirectory,
    OzAddHeapSize,
    OzIoControl,
    OzSeek,
    OzMap,
};

u64int nsyscalls = N_SYSCALLS;

void syscallHandler(struct RegisterState *regs)
{
    void *func;

    if (regs->rax < nsyscalls) {
        func = syscalls[regs->rax];
    } else {
        regs->rax = 0xffffffff;
        return;
    }

    u64int ret;
    asm volatile("push %1;  \
                  push %2;  \
                  push %3;  \
                  push %4;  \
                  push %5;  \
                  call *%6; \
                  pop %%rbx;\
                  pop %%rbx;\
                  pop %%rbx;\
                  pop %%rbx;\
                  pop %%rbx;"
                  : "=a"(ret) : "D"(regs->rbx), "S"(regs->rcx), "d"(regs->rdx), 
                    "c"(regs->rsi), "b"(regs->rdi), "r"(func));


//    asm volatile("xchg %bx,%bx");
//    DBG("func:%d, rax:%x", regs->rax, ret);
    regs->rax = ret;
}

void initSyscalls()
{
    registerInterruptHandler(128, &syscallHandler);
}

void syscallTest()
{
    PANIC("syscall happends");
}



