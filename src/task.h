#ifndef TASK_H
#define TASK_H 

#include "common.h"
#include "paging.h"
#include "isr.h"
#include "vm.h"

#define KERNEL_STACK_START 0x40000000
#define KERNEL_STACK_SIZE 0x1000

#define USER_STACK_START 0xd0000000
#define USER_STACK_SIZE 0x1000

#define USER_ADDR_START 0x40000000

#define TASK_READY    0
#define TASK_INTR_WAIT  1
#define TASK_UNINTR_WAIT  2
#define TASK_ZOMBIE   3

#define MAX_TASK_FDS  256
#define MAX_MMAP_LENGTH 0x10000000

typedef struct task {
    char path[MAX_PATH_LEN];
    u32int pid;
    u32int ppid;
    u32int state;
    u32int ret_code;
    u32int eip;
    u32int ebp;
    u32int esp;
    page_directory_t *dir;
    file_t *file;
    vm_t *vm;
    u32int kstack;
    file_t *fd[MAX_TASK_FDS];
    struct task *next;
} task_t;

typedef struct timer_queue_struct {
    u32int expires;
    task_t *task;
    struct timer_queue_struct *next;
    struct timer_queue_struct *prev;
} timer_queue_t;

typedef struct wait_queue_node_struct {
    task_t *task;
    struct wait_queue_node_struct *next;
    struct wait_queue_node_struct *prev;
} wait_queue_node_t;

typedef struct {
    wait_queue_node_t *head;
} wait_queue_t;

extern timer_queue_t *timer_queue;

extern task_t *current_task;

void init_multitasking();

void handle_timer_queue();

void wake_up_one(wait_queue_t *wq);

void wake_up_all(wait_queue_t *wq);

void sleep_for_ticks(u32int sleep_ticks);

void sleep_on(wait_queue_t *wq);

u32int fork();

s32int sys_execve(char *path, char **argv, char **envp);

s32int execve(char *path, char **argv, char **envp);

void exit(s32int ret_code);

void switch_task();

void switch_to_user_mode();

u32int getpid();

s32int handle_user_page_fault(u32int fault_addr, u32int rw);

void general_protection_handler(registers_t *regs);

s32int copy_from_user(void *kbuf, void *ubuf, u32int n);

s32int copy_to_user(void *ubuf, void *kbuf, u32int n);

u32int sys_mmap(u32int addr, u32int length, s32int flags,
        s32int fd, u32int offset);

s32int sys_unmap(u32int addr, u32int length);

u32int sys_msleep(u32int msec);


#endif /* TASK_H */
