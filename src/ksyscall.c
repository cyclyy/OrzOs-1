#include "ksyscall.h"
#include "interrupt.h"
#include "util.h"
#include "task.h"
#include "vfs.h"
#include "debugcon.h"
#include "syscall/ozipc.h"
#include "syscall/oztask.h"
#include "syscall/ozfs.h"
#include "syscall/oztime.h"

void syscallTest();

#define N_SYSCALLS 22

void *syscalls[N_SYSCALLS] = {
    &syscallTest,
    &dbgPutch,
    OzPost,
    OzSend,
    OzReceive,
    OzGetPid,
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
    OzReadAsync,
    OzMilliAlarm,
    OzMilliSleep,
    OzSendReceive,
    OzReply,
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
    asm volatile("push %%r8;  \
                  mov %%r8, %%rbx; \
                  call *%%rax; \
                  pop %%r8;"
                  : "=a"(ret) : "D"(regs->rbx), "S"(regs->rcx), "d"(regs->rdx), 
                    "c"(regs->rsi), "b"(regs->rdi), "a"(func));


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



