#include "syscalls.h"
#include "isr.h"
#include "screen.h"
#include "task.h"
#include "vfs.h"

void _syscall_test();

#define N_SYSCALLS (21)

void *syscalls[N_SYSCALLS] = {
    &fork,
    &sys_execve,
    &exit,
    &sys_fdopen,
    &sys_fdclose,
    &sys_fdread,
    &sys_fdwrite,
    &sys_fdlseek,
    &sys_mmap,
    &sys_unmap,
    &sys_msleep,
    &sys_mkdir,
    &sys_mknod,
    &sys_create,
    &sys_rmdir,
    &sys_rm,
    &sys_rename,
    &sys_getdents,
    &sys_getcwd,
    &sys_chdir,
    &scr_putch,
};

u32int nsyscalls = N_SYSCALLS;

void syscall_handler(registers_t *regs)
{
    void *func;

    if (regs->eax < nsyscalls) {
        func = syscalls[regs->eax];
    } else {
        regs->eax = 0xffffffff;
        return;
    }

    u32int ret;
    asm volatile("push %1;  \
                  push %2;  \
                  push %3;  \
                  push %4;  \
                  push %5;  \
                  call *%6; \
                  pop %%ebx;\
                  pop %%ebx;\
                  pop %%ebx;\
                  pop %%ebx;\
                  pop %%ebx;"
                  : "=a"(ret) : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), 
                    "r"(regs->ecx), "r"(regs->ebx), "r"(func));


//    asm volatile("xchg %bx,%bx");
    regs->eax = ret;
}

void init_syscalls()
{
    register_interrupt_handler(128, &syscall_handler);
}

void _syscall_test()
{
    PANIC("syscall happends");
}



