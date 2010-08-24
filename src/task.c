#include "task.h"
#include "kheap.h"
#include "paging.h"
#include "desc_table.h"
#include "isr.h"
#include "screen.h"
#include "vm.h"
#include "vfs.h"
#include "elf.h"
#include "timer.h"

task_t *current_task = 0;
timer_queue_t *timer_queue = 0;
task_t *task_queue = 0;

extern tss_entry_t tss0;

extern u32int initial_esp;

extern page_directory_t *current_dir;
extern page_directory_t *kernel_dir;

extern u32int read_eip();

extern void copy_physical_frame(u32int src, u32int dst);
extern page_t *get_page(u32int vaddr, u32int make, page_directory_t *dir);

u32int last_pid = 1;

u32int load_program(file_t *f, vm_t *vm);

page_directory_t* clone_task_directory(page_directory_t *dir, vm_t *vm)
{
    page_directory_t *new_dir = clone_directory(kernel_dir);

    u32int i;
    page_t *page;
    page_t *new_page;

    // copy kernel stack;
    for (i=KERNEL_STACK_START-KERNEL_STACK_SIZE; i<KERNEL_STACK_START; i+=0x1000) {
        new_page = get_page(i, 1, new_dir);
        alloc_frame(new_page, 1, 1);
        page = get_page(i, 0, dir);
        /*
         *printk("virt addr %p\n", i);
         *printk("copy phys frame %p->%p\n", page->addr*0x1000, new_page->addr*0x1000);
         */
        copy_physical_frame(page->addr*0x1000, new_page->addr*0x1000);
    }

    if (!vm) {
        /*printk("clone kernel thread.\n");*/
        return new_dir;
    }

    vma_t *vma = vm->vmas;

    while (vma) {
        for (i=vma->start; i<vma->end; i+=0x1000) {
            page = get_page(i, 0, dir);
            if (page) {
                new_page = get_page(i, 1, new_dir);
                if (page->addr && page->present) {
                    alloc_frame(new_page, page->us, page->rw);
                    copy_physical_frame(page->addr*0x1000, new_page->addr*0x1000);
                }
            }
        }
        vma = vma->next;
    }

    return new_dir;
}

/*
void copy_kernel_stack(u32int src, page_directory_t *src_dir, u32int dst, page_directory_t *dst_dir)
{
    u32int i, offset;
    for (i=0; i<KERNEL_STACK_SIZE; i+=0x1000) {
        copy_page( get_page(src+i,0,src_dir),  get_page(dst+i,0,dst_dir) );
    }

    offset = dst - src;

    // fix ebp
    for (i=0; i<KERNEL_STACK_SIZE; i+=0x4) {
        u32int addr = *(u32int *)(dst+i);
        if ((addr >= src) && (addr < src+KERNEL_STACK_SIZE)) {
            addr += offset;
            *(u32int *)(dst+i) = addr;
        }
    }
}
*/

void move_stack(u32int new_start, u32int sz)
{
//    scr_putp("initial_esp",initial_esp);
    u32int offset;
    u32int old_esp, old_ebp;
    u32int new_esp, new_ebp;
    u32int i;

//    scr_puts("move_stack\n");
    // init address space
    for (i=new_start-sz; i<new_start; i+=0x1000) {
//        scr_putp("get_page",i);
        page_t *page = get_page(i, 1, current_dir);
//        scr_puts("alloc_frame\n");
        alloc_frame(page, 1, 1);
    }

//    asm volatile("xchg %bx, %bx");
    // Flush the TLB by reading and writing the page directory address again.
    u32int pd_addr;
    asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
    asm volatile("mov %0, %%cr3" : : "r" (pd_addr)); 

    asm volatile("mov %%esp, %0" : "=r"(old_esp));
    asm volatile("mov %%ebp, %0" : "=r"(old_ebp));

    offset = new_start - initial_esp;

    new_esp = old_esp + offset;
    new_ebp = old_ebp + offset;

//    scr_putp("old_esp",old_esp);
//    scr_putp("old_ebp",old_ebp);
//    scr_putp("new_esp",new_esp);
//    scr_putp("new_ebp",new_ebp);
    memcpy((void*)new_esp, (void*)old_esp, initial_esp - old_esp);

    // fool walk the stack to correct ebp ;
    for (i=new_esp; i<new_start; i+=4) {
        u32int addr = *(u32int *)i;
        if ((old_esp<=addr) && (addr<initial_esp)) {
            u32int new_addr = addr + offset;
            *(u32int *)i = new_addr;
        }
    }

    asm volatile("mov %0, %%esp" :: "r"(new_esp));
    asm volatile("mov %0, %%ebp" :: "r"(new_ebp));
}

void init_multitasking()
{
    asm volatile("cli");

    move_stack(KERNEL_STACK_START,KERNEL_STACK_SIZE);

    /*
    u32int old_ebp, old_esp;

    asm volatile("mov %%esp, %0" : "=r"(old_esp));
    scr_putp("esp after move_stack",old_esp);
    asm volatile("mov %%ebp, %0" : "=r"(old_ebp));
    scr_putp("ebp after move_stack",old_ebp);
    */

    current_task = task_queue = (task_t*)kmalloc(sizeof(task_t));
    memset(current_task, 0, sizeof(task_t));

    ASSERT(current_task);

    memset(current_task, 0, sizeof(task_t));

    current_task->pid = last_pid++;
    current_task->ppid = 0;
    current_task->state = TASK_READY;
    current_task->dir = current_dir;


    set_kernel_stack(KERNEL_STACK_START);

    asm volatile("mov %%esp, %0" : "=r"(current_task->esp));
    asm volatile("mov %%ebp, %0" : "=r"(current_task->ebp));

    register_interrupt_handler(13, &general_protection_handler);

    asm volatile("sti");
}

void handle_timer_queue()
{
    if (!timer_queue)
        return;
        
    timer_queue_t *q = timer_queue;
    timer_queue_t *tmp;
    q->expires--;
    while (q && (q->expires == 0)) {
        q->task->state = TASK_READY;
        if (q->prev) {
            q->prev->next = q->next;
        } else {
            timer_queue = q->next;
        }
        if (q->next) {
            q->next->prev = q->prev;
        }
        tmp = q;
        q = q->next;
        kfree(tmp);
    }
}

void wake_up_one(wait_queue_t *wq)
{
    if (!wq || !wq->head)
        return;

    wait_queue_node_t *p = wq->head;
    p->task->state = TASK_READY;
    wq->head = p->next;
    kfree(p);
} 

void wake_up_all(wait_queue_t *wq)
{
    while (wq->head)
        wake_up_one(wq);
} 

void sleep_for_ticks(u32int sleep_ticks)
{
    if (!current_task || !sleep_ticks)
        return;

    timer_queue_t *q = 0;
    timer_queue_t *me = (timer_queue_t*)kmalloc(sizeof(timer_queue_t));
    memset(me, 0, sizeof(timer_queue_t));
    me->task = current_task;
    me->expires = sleep_ticks;
    if (timer_queue) {
        q = timer_queue;
        do {
            if (sleep_ticks <= q->expires) {
                me->expires = sleep_ticks;
                q->expires -= sleep_ticks;
                if (q->prev) {
                    q->prev->next = me;
                } else {
                    timer_queue = me;
                }
                me->next = q;
                me->prev = q->prev;
                q->prev = me;
                break;
            } else {
                sleep_ticks -= q->expires;
                if (q->next) {
                    q = q->next;
                } else {
                    me->expires = sleep_ticks;
                    q->next = me;
                    me->prev = q;
                    break;
                }
            }
        } while(1);
    } else {
        timer_queue = me;
    }

    current_task->state = TASK_UNINTR_WAIT;

    switch_task();
}

u32int fork()
{
    asm volatile("cli");

    scr_puts("fork\n");

    task_t *parent_task = current_task;

    page_directory_t *dir = clone_task_directory(current_task->dir, current_task->vm);
    vm_t *vm = clone_vm(current_task->vm);

    task_t *child_task = (task_t *)kmalloc(sizeof(task_t));
    memcpy(child_task, current_task, sizeof(task_t));

    child_task->pid = last_pid++;
    child_task->ppid = parent_task->pid;
    child_task->state = TASK_READY;
    child_task->dir = dir;
    child_task->vm = vm;
    child_task->next = 0;

    //child_task->kstack = kmalloc_a(KERNEL_STACK_SIZE);
    /*child_task->kstack = KERNEL_STACK_START - KERNEL_STACK_SIZE;*/

    task_t *t = task_queue;
    while (t->next)
        t = t->next;
    t->next = child_task;


    u32int eip, esp, ebp;

    eip = read_eip();

//    scr_putp("current_task",current_task);
//    scr_putp("parent_task",parent_task);
//    u32int esp; 
//    asm volatile("mov %%esp, %0" : "=r"(esp));
//    scr_putp("esp",esp);

    // eip is also the execution start address of child task;

    if (current_task == parent_task)  {
        /*printk("i'm parent %p\n", current_task->pid);*/
        child_task->eip = eip;

        //copy_kernel_stack(current_task->kstack, current_task->dir, child_task->kstack, child_task->dir);

        asm volatile("mov %%esp, %0" : "=r"(esp) );
        asm volatile("mov %%ebp, %0" : "=r"(ebp) );

        page_t *p = get_page(esp, 0, current_task->dir);
        page_t *q = get_page(esp, 0, child_task->dir);

        child_task->esp = esp;
        child_task->ebp = ebp;

        asm volatile("sti");

        return child_task->pid;
    } else {
        /*asm volatile("xchg %bx,%bx");*/
        /*printk("i'm child %p\n", current_task->pid);*/
        return 0;
    }
}

void switch_task()
{
    asm volatile("cli");
    if (current_task == 0)
        return;

    u32int esp, ebp, eip;

    asm volatile("mov %%esp, %0" : "=r"(esp) );
    asm volatile("mov %%ebp, %0" : "=r"(ebp) );

    eip = read_eip();

//    scr_puts("switch_task.");
    /*
       1) switched to this task "jmp task->eip"
       2) normal running
     */
    if (eip == 0x123)  {
//        asm volatile("cli");
//        scr_puts("switch_task succeeds.");
//        asm volatile("sti");
//    asm volatile("xchg %bx, %bx");
        return;
    }

    current_task->esp = esp;
    current_task->ebp = ebp;
    current_task->eip = eip;

    do {
        current_task = current_task->next;

        if (current_task == 0)
            current_task = task_queue;
    } while (current_task->state != TASK_READY);

    esp = current_task->esp;
    ebp = current_task->ebp;
    eip = current_task->eip;

    // update the current page directory
    current_dir = current_task->dir;

    // update tss0.esp0
    /*set_kernel_stack(current_task->kstack+KERNEL_STACK_SIZE);*/

//    scr_putp("switch_task to pid", current_task->pid);
//    scr_putp("switch_task to dir", current_task->dir->addr);
    /*printk("switch to task %d\n", current_task->pid);*/

    asm volatile("cli;                  \
                  mov %0, %%esp;        \
                  mov %1, %%ebp;        \
                  mov %2, %%ecx;       \
                  mov %3, %%cr3;        \
                  movl $0x123, %%eax;    \
                  sti;                  \
                  jmp *%%ecx"                    
                  ::"r"(esp), 
                    "r"(ebp), 
                    "r"(eip),
                    "r"(current_task->dir->addr));

}

void switch_to_user_mode()
{
    //set_kernel_stack(current_task->kstack+KERNEL_STACK_SIZE);

    // Set up a stack structure for switching to user mode.
    asm volatile("  \
            cli; \
            mov $0x23, %ax; \
            mov %ax, %ds; \
            mov %ax, %es; \
            mov %ax, %fs; \
            mov %ax, %gs; \
            \
            mov %esp, %eax; \
            pushl $0x23; \
            pushl %eax; \
            pushf; \
            \
            pop %eax; \
            or $0x200, %eax; \
            push %eax; \
            \
            pushl $0x1B; \
            push $1f; \
            iret; \
            1: \
            ");
}

s32int sys_execve(char *path, char **argv, char **envp)
{
    /*
     *must alloc _path on stack, 
     *since execve never return when success,
     *can't kfree _path,
     *but execve use same kernel stack,
     *so _path auto freed.
     */
    char _path[MAX_PATH_LEN];
    u32int n = copy_from_user(_path, path, MAX_PATH_LEN);
    return execve(_path, argv, envp);
}

s32int execve(char *path, char **argv, char **envp)
{
    asm volatile("cli");

    file_t *f = file_open(path, 0);

    vm_t *vm = (vm_t*)kmalloc(sizeof(vm_t));
    memset(vm, 0, sizeof(vm_t));

    page_directory_t *dir = clone_directory(current_dir);

    load_program(f, vm);

    // create user stack;
    vma_t *vma = (vma_t*)kmalloc(sizeof(vma_t));
    memset(vma, 0, sizeof(vma_t));
    vma->start = USER_STACK_START - USER_STACK_SIZE;
    vma->end = USER_STACK_START;
    vma->flags |= VMA_READ | VMA_WRITE | VMA_STACK | VMA_ANON;
    insert_vma(vm, vma);
    vm->start_stack = vma->end;
    vm->end_stack = vma->start;
    /*printk("stack %p->%p\n", vma->start, vma->end);*/

    // release current task's vm
    if (current_task->vm) {
        free_vmas(current_task->vm);
        kfree(current_task->vm);
    }

    strcpy(current_task->path, path);

    current_task->file = f;

    current_task->vm = vm;
    current_task->dir = dir;

    current_task->eip = current_task->vm->entry;
    current_task->ebp = current_task->vm->start_stack;
    current_task->esp = current_task->vm->end_stack;

    current_task->dir = current_task->dir;
    current_dir = current_task->dir;

    /*printk("current_task->vm->entry %p\n", current_task->vm->entry);*/
    /*printk("current_task->dir: %p\n", current_task->dir);*/
    /*printk("current_task->vm->start_stack: %p\n", current_task->vm->start_stack);*/
    /*printk("current_task->eip: %p\n", current_task->eip);*/
    /*printk("current_task->kstack: %p\n", current_task->kstack);*/
    /*printk("tss0.esp0: %p\n", tss0.esp0);*/

    asm volatile("  \
            mov $0x23, %%ax; \
            mov %%ax, %%ds; \
            mov %%ax, %%es; \
            mov %%ax, %%fs; \
            mov %%ax, %%gs; \
            \
            movl %0, %%cr3; \
            \
            pushl $0x23; \
            pushl %1; \
            pushf; \
            \
            pop %%eax; \
            or $0x200, %%eax; \
            push %%eax; \
            \
            pushl $0x1b; \
            push %2; \
            iret; \
            1: \
            "::"c"(current_task->dir->addr), "b"(current_task->vm->start_stack), "d"(current_task->eip));

    return -EFAULT;
}

u32int getpid()
{
    if (current_task)
        return current_task->pid;

    return 0;
}

void general_protection_handler(registers_t *regs)
{
    printk("general protection error %p\n", regs->eip);
    PANIC("General protection error");
}

// page fault while access user space address
s32int handle_user_page_fault(u32int fault_addr, u32int rw)
{
    if (fault_addr < USER_ADDR_START) {
        /*printk("Page fault: user trying to access kernel mem\n");*/
        return -PFAULT_EACCESS;
    }

    vma_t *vma = get_vma_cover(current_task->vm, fault_addr);

    if (!vma) {
        /*printk("Page fault: user trying to access non-mapped user area\n");*/
        return -PFAULT_EACCESS;
    }

    page_t *p = get_page(fault_addr, 0, current_task->dir);

   
    if (!p || !p->addr) {
        vma_nopage(vma, fault_addr & 0xfffff000);
        return PFAULT_HANDLED;
    } else if (!p->present) {
        // maybe paged out to disk
        PANIC("TODO: page swap in.");
    } else if (rw) {
        if ((vma->flags & VMA_WRITE) && (p->rw==0)) {
            PANIC("TODO: implement copy on write");
            return PFAULT_HANDLED;
        } else if ((vma->flags & VMA_WRITE) == 0) {
            // read only page:
            printk("Page fault: user trying to write to readonly pages\n");
            return -PFAULT_ERDONLY;
        }
    }

    return PFAULT_HANDLED;
}

u32int verify_program_header(Elf32_Ehdr *header) 
{
    if (!header)
        return 0;

    u32int ok = 1;

    ok &= (header->e_ident[EI_MAG0] == ELFMAG0);
    ok &= (header->e_ident[EI_MAG1] == ELFMAG1);
    ok &= (header->e_ident[EI_MAG2] == ELFMAG2);
    ok &= (header->e_ident[EI_MAG3] == ELFMAG3);
    ok &= (header->e_ident[EI_CLASS] == ELFCLASS32);
    ok &= (header->e_ident[EI_DATA] == ELFDATA2LSB);
    ok &= (header->e_ident[EI_VERSION] == EV_CURRENT);

    ok &= (header->e_type == ET_EXEC);
    ok &= (header->e_machine == EM_386);
    ok &= (header->e_entry >= 0x40000000);
    ok &= (header->e_phnum > 0);

    return ok;
}

u32int load_program(file_t *f, vm_t *vm)
{
    if (!vm)
        return 0;

    file_lseek(f,0,SEEK_SET);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)kmalloc(sizeof(Elf32_Ehdr));
    u32int n = file_read(f, ehdr, sizeof(Elf32_Ehdr));

    if ( (n == sizeof(Elf32_Ehdr)) && verify_program_header(ehdr) ) {
        vm->entry = ehdr->e_entry;

        file_lseek(f,ehdr->e_phoff,SEEK_SET);
        Elf32_Phdr *phdr = (Elf32_Phdr *)kmalloc(ehdr->e_phentsize * ehdr->e_phnum);
        n = file_read(f, phdr, ehdr->e_phentsize * ehdr->e_phnum);
        /*printk("phdr[n]: %p\n", &phdr[0]);*/
        u32int i;
        for (i=0; i<ehdr->e_phnum; i++) {
            if (phdr[i].p_type == PT_LOAD) {
                vma_t *vma = (vma_t*)kmalloc(sizeof(vma_t));
                memset(vma, 0, sizeof(vma_t));
                vma->start = phdr[i].p_vaddr;
                vma->end = phdr[i].p_vaddr + phdr[i].p_memsz;
                if (vma->end & 0xfff) {
                    vma->end = (vma->end & 0xfffff000) + 0x1000;
                }

                file_mapping_t *f_map = (file_mapping_t*)kmalloc(sizeof(file_mapping_t));
                f_map->file = f;
                f_map->offset = phdr[i].p_offset;
                f_map->size = phdr[i].p_filesz;
                vma->priv = f_map;

                vma->flags = VMA_FILE | VMA_PRIVATE;
                if (phdr[i].p_flags & PF_R)
                    vma->flags |= VMA_READ;
                if (phdr[i].p_flags & PF_W)
                    vma->flags |= VMA_WRITE;
                if (phdr[i].p_flags & PF_X) 
                    vma->flags |= VMA_EXEC;

                if (vm->start_brk < vma->end)
                    vm->start_brk = vma->end;

                insert_vma(vm, vma);
            }
        }

        vm->start_brk &= PAGE_MASK;
        vm->start_brk += PAGE_SIZE;
        vm->brk = vm->start_brk;

    } else {
        return 0;
    }

    return 1;
}

void exit(s32int ret)
{
    printk("%s ret: %d\n", __FUNCTION__, ret);
    if (current_task->pid==1)
        PANIC("Attempting to kill init");

    current_task->state = TASK_ZOMBIE;
    current_task->ret_code = ret;

    // TODO: release current_task's resource 

    switch_task();
}


s32int copy_from_user(void *kbuf, void *ubuf, u32int n)
{
    u32int uaddr = (u32int)ubuf;
    u32int len;
    u32int copied = 0;

    do {
        if (handle_user_page_fault(uaddr & PAGE_MASK, 0) == PFAULT_HANDLED) {
            len = MIN(PAGE_SIZE - (uaddr & PAGE_MASK), n);
            memcpy(kbuf + copied, ubuf + copied, len);
            n -= len;
            copied += len;
            uaddr += len;
        } else {
            return copied;
        }
    } while (n);

    return copied;
}

s32int copy_to_user(void *ubuf, void *kbuf, u32int n)
{
    u32int uaddr = (u32int)ubuf;
    u32int len;
    u32int copied = 0;

    do {
        if (handle_user_page_fault(uaddr & PAGE_MASK, 1) == PFAULT_HANDLED) {
            len = MIN(PAGE_SIZE - (uaddr & PAGE_MASK), n);
            memcpy(ubuf + copied, kbuf + copied, len);
            n -= len;
            copied += len;
            uaddr += len;
        } else {
            return copied;
        }
    } while (n);

    return copied;
}

u32int sys_mmap(u32int addr, u32int length, s32int flags,
        s32int fd, u32int offset)
{
    if (!current_task->vm)
        return -1;

    if (length > MAX_MMAP_LENGTH)
        return -1;

    u32int brk = current_task->vm->brk;
    u32int ma = addr;

    if (ma < brk)
        ma = brk;

    // up overflow
    if (ma + length<brk)
        ma = brk;

    u32int pa = get_vm_free_start(current_task->vm, ma, length);

    printk("mmap [%p,%p) get %p\n", addr, length, pa);

    if (pa == 0)
        return -1;

    vma_t *vma = (vma_t*)kmalloc(sizeof(vma_t));
    vma->start = pa;
    vma->end = pa+length;
    vma->flags = flags;
    vma->priv = 0;
    if (flags & VMA_FILE) {
        file_mapping_t *fmap = (file_mapping_t*)kmalloc(sizeof(file_mapping_t));
        fmap->offset = offset;
        fmap->size = length;
        fmap->fd = fd;
        fmap->file = current_task->fd[fd];
        vma->priv = fmap;
    }
    insert_vma(current_task->vm, vma);

    return pa;
}

s32int sys_unmap(u32int addr, u32int length)
{
    vma_t *vma = get_vma_cover2(current_task->vm, addr, addr+length);

    if (vma) {
        remove_vma(current_task->vm, vma);
        vma_freepages(vma);
        return 0;
    } else {
        return -1;
    }
}

u32int sys_msleep(u32int msec)
{
    sleep_for_ticks(msec/(1000/HZ));
    return 0;
}

void sleep_on(wait_queue_t *wq)
{
    wait_queue_node_t *p = (wait_queue_node_t *)kmalloc(sizeof(wait_queue_node_t));
    p->task = current_task;
    p->next = p->prev = 0;
    if (!wq->head) {
        wq->head = p;
    } else {
        wait_queue_node_t *q = wq->head;
        while (q->next)
            q = q->next;
        q->next = p;
        p->prev = q;
    }
    current_task->state = TASK_UNINTR_WAIT;
    switch_task();
}


