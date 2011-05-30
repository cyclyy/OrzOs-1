#include "ksyscall.h"
#include "interrupt.h"
#include "util.h"
#include "task.h"
#include "vfs.h"
#include "screen.h"

void syscallTest();

#define N_SYSCALLS 2

void *syscalls[N_SYSCALLS] = {
    &syscallTest,
    &scr_putch,
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
                  : "=a"(ret) : "D"(regs->rdi), "S"(regs->rsi), "d"(regs->rdx), 
                    "c"(regs->rcx), "b"(regs->rbx), "r"(func));


//    asm volatile("xchg %bx,%bx");
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


