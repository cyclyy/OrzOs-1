#include "task.h"
#include "schedule.h"
#include "sysdef.h"
#include "kmm.h"
#include "vmm.h"
#include "util.h"
#include "bootinfo.h"
#include "interrupt.h"
#include "program.h"
#include "elfloader.h"
#include "vfs.h"

struct Task *taskQueue = 0;
struct Task *deadQueue = 0;
struct Task *currentTask = 0;

u64int lastPid = 0;

void moveKernelStack()
{
    u64int offset;
    u64int oldRsp, oldRbp;
    u64int newRsp, newRbp;
    u64int i;
    u64int paddr, addr;

    kMallocEx(KERNEL_STACK_SIZE,1,&paddr);

    if (!paddr)
        printk("FAILED TO ALLOC MEM");

    mapPagesVtoP(KERNEL_STACK_TOP-KERNEL_STACK_SIZE, 
            paddr, 
            KERNEL_STACK_SIZE/PAGE_SIZE, 
            getPML4E(), 1);

    offset = KERNEL_STACK_TOP - getBootInfo()->initialStack;

    asm("mov %%rsp, %0" : "=r"(oldRsp));
    asm("mov %%rbp, %0" : "=r"(oldRbp));

    newRsp = oldRsp + offset;
    newRbp = oldRbp + offset;

    memcpy((void*)newRsp, (void*)oldRsp, getBootInfo()->initialStack - oldRsp);

    // fool walk the stack to correct rbp ;
    for (i=newRsp; i<KERNEL_STACK_TOP; i+=PTR_SIZE) {
        addr = *(u64int *)i;
        if ((oldRsp<=addr) && (addr<getBootInfo()->initialStack)) {
            *(u64int *)i = addr + offset;
        }
    }

    asm("mov %0, %%rsp" :: "r"(newRsp));
    asm("mov %0, %%rbp" :: "r"(newRbp));
}

void timerHandler(struct RegisterState *rs)
{
    ++currentTask->ticks;
    --currentTask->slices;
    if (!currentTask->slices) {
        schedule();
    }
}

void initMultitasking()
{
    struct Task *task;

    moveKernelStack();

    task = (struct Task *)kMalloc(sizeof(struct Task));
    memset(task, 0, sizeof(struct Task));
    task->state = TASK_STATE_READY;
    task->vm = vmInit();

    currentTask = task;
    //memset(runQueue,0,PTR_SIZE*MLFQ_LEVELS);
    //runQueue[0] = currentTask;
    //currentTask->rqNext = currentTask->rqPrev = currentTask;
    rqAdd(currentTask);

    taskQueue = currentTask;

    registerInterruptHandler(IRQ0, timerHandler);
}

void startUserThread()
{
    asm ("  \
            mov $0x3, %%ax; \
            mov %%ax, %%ds; \
            mov %%ax, %%es; \
            mov %%ax, %%fs; \
            mov %%ax, %%gs; \
            \
            pushq $0x23; \
            pushq %0; \
            pushfq; \
            \
            pop %%rax; \
            or $0x200, %%rax; \
            pushq %%rax; \
            \
            pushq $0x1b; \
            pushq %1; \
            iretq; \
            "::"b"(currentTask->rsp3), "d"(currentTask->rip3));

    for(;;);

}

void startKernelThread()
{
    s64int (*func)();
    func = (s64int (*)())currentTask->rip3;
    func();
    kExitTask(0);
}

s64int kNewKernelThread(void *entry)
{
    struct Task *newTask;
    struct VMA *vma;
    u64int i;

    asm("cli");
    newTask = (struct Task *)kMalloc(sizeof(struct Task));
    memset(newTask, 0, sizeof(struct Task));
    newTask->state = TASK_STATE_READY;
    newTask->priority = 0;
    newTask->pid = ++lastPid;
    newTask->prog = 0;
    newTask->vm = vmRef(kernelVM);
    newTask->rip = (u64int)&startKernelThread;
    newTask->rip3 = (u64int)entry;
    for (i=KERNEL_STACK_TOP; i>KERNEL_STACK_BOTTOM; i-=2*KERNEL_STACK_SIZE) {
        vma = vmQueryArea(kernelVM, i-KERNEL_STACK_SIZE);
        if (vma && (vma->flags & VMA_STATUS_FREE)) {
            vmAddArea(kernelVM, i-KERNEL_STACK_SIZE, KERNEL_STACK_SIZE, 
                    VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_STACK);
            newTask->rsp0 = newTask->rbp = newTask->rsp = i;
            break;
        }
    }
    if (!newTask->rsp0) 
        goto cleanup;

    if (taskQueue)
        taskQueue->prev = newTask;
    newTask->next = taskQueue;
    taskQueue = newTask;

    rqAdd(newTask);

    return 0;

cleanup:
    kFree(newTask);

    return -1;
}

s64int kNewThread(void *entry)
{
    struct Task *newTask;
    struct VMA *vma;
    u64int i;

    asm("cli");
    newTask = (struct Task *)kMalloc(sizeof(struct Task));
    memset(newTask, 0, sizeof(struct Task));
    newTask->state = TASK_STATE_READY;
    newTask->priority = 0;
    newTask->pid = ++lastPid;
    newTask->prog = currentTask->prog;
    newTask->vm = vmRef(currentTask->vm);
    newTask->rip = (u64int)&startUserThread;
    newTask->rip3 = (u64int)entry;
    for (i=KERNEL_STACK_TOP; i>KERNEL_STACK_BOTTOM; i-=2*KERNEL_STACK_SIZE) {
        vma = vmQueryArea(newTask->vm, i-KERNEL_STACK_SIZE);
        if (vma && (vma->flags & VMA_STATUS_FREE)) {
            vmAddArea(newTask->vm, i-KERNEL_STACK_SIZE, KERNEL_STACK_SIZE, 
                    VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_STACK);
            newTask->rsp0 = newTask->rsp = newTask->rbp = i;
            break;
        }
    }
    if (!newTask->rsp0) 
        goto cleanup;

    for (i=USER_STACK_TOP; i>USER_STACK_BOTTOM; i-=2*USER_STACK_SIZE) {
        vma = vmQueryArea(newTask->vm, i-USER_STACK_SIZE);
        if (vma && (vma->flags & VMA_STATUS_FREE)) {
            vmAddArea(newTask->vm, i-USER_STACK_SIZE, USER_STACK_SIZE, 
                    VMA_STATUS_USED | VMA_OWNER_USER | VMA_TYPE_STACK);
            newTask->rsp3 = i;
            break;
        }
    }
    if (!newTask->rsp3) 
        goto cleanup;

    if (taskQueue)
        taskQueue->prev = newTask;
    newTask->next = taskQueue;
    taskQueue = newTask;

    rqAdd(newTask);

    return 0;

cleanup:
    kFree(newTask);

    return -1;
}

s64int kNewTask(const char *path, u64int flags)
{
    struct Task *newTask;
    s64int ret;

    asm("cli");
    newTask = (struct Task *)kMalloc(sizeof(struct Task));
    memset(newTask, 0, sizeof(struct Task));
    newTask->state = TASK_STATE_READY;
    newTask->priority = 0;
    newTask->pid = ++lastPid;
    newTask->rip = (u64int)&startUserThread;
    newTask->rbp = KERNEL_STACK_TOP;
    newTask->rsp = KERNEL_STACK_TOP;
    newTask->rsp0 = KERNEL_STACK_TOP;
    newTask->vm = vmCreate();
    newTask->prog = (struct Program*)kMalloc(sizeof(struct Program));
    memset(newTask->prog, 0, sizeof(struct Program));
    ret = loadElfProgram(path, newTask->prog, newTask->vm);
    newTask->rsp3 = USER_STACK_TOP;
    newTask->rip3 = newTask->prog->entry;

    if (ret!=0)
        goto cleanup;

    if (taskQueue)
        taskQueue->prev = newTask;
    newTask->next = taskQueue;
    taskQueue = newTask;

    rqAdd(newTask);

    return 0;

cleanup:
    kFree(newTask->prog);
    kFree(newTask);

    return -1;
}

void kExitTask(s64int exitCode)
{
    asm("cli");
    if (currentTask->pid==0)
        PANIC("You can't terminate root task.");

    currentTask->exitCode = exitCode;
    currentTask->state = TASK_STATE_DEAD;
    rqRemove(currentTask);
    currentTask->sqNext = deadQueue;
    currentTask->sqPrev = 0;
    if (deadQueue) 
        deadQueue->sqPrev = currentTask;
    deadQueue = currentTask;
    schedule();
}

void testThread()
{
    printk("PID:%d\n",currentTask->pid);
}

void rootTask()
{
    struct Task *t;
    int i;

    for (i=0; i<3; i++)
        kNewKernelThread(testThread);

    while (1) {
        asm("cli");
        currentTask->slices = 0;
        while (deadQueue) {
            t = deadQueue;
            if (t->prev)
                t->prev->next = t->next;
            if (t->next)
                t->next->prev = t->prev;
            vmDeref(t->vm);
            kFree(t->prog);
            kFree(t);
            deadQueue = deadQueue->sqNext;
        }
        schedule();
    }
}

