#include "paging.h"
#include "isr.h"
#include "screen.h"
#include "kheap.h"
#include "task.h"

#define IDX_FROM_BITS(x) ( x / (4*8) )
#define OFF_FROM_BITS(x) ( x % (4*8) )

extern u32int placement_addr;
extern heap_t *kheap;

extern void copy_physical_frame(u32int src, u32int dst);

u32int *frames = 0;
u32int nframes = 0;

page_directory_t *kernel_dir = 0;
page_directory_t *current_dir = 0;

void page_fault_handler(registers_t *regs);

void alloc_frame(page_t *page, u32int kernel, u32int writable);

u32int test_frame(u32int frame_addr);
void set_frame(u32int frame_addr);
void clear_frame(u32int frame_addr);
u32int first_frame();

void init_paging()
{
    // 16 mb memory for paging
    u32int mem_for_paging = 0x1000000;
    nframes = mem_for_paging / 0x1000;

    kernel_dir = (page_directory_t *)kmalloc_a(sizeof(page_directory_t));
    memset(kernel_dir, 0, sizeof(page_directory_t));
    current_dir = kernel_dir;

    kernel_dir->addr = (u32int)&kernel_dir->table_addr;

    u32int nbytes = IDX_FROM_BITS(nframes) * sizeof(u32int);
    frames = (u32int *)kmalloc(nbytes);
    memset(frames, 0, nbytes);

    u32int i;

    page_t *page = 0;

    u32int kheap_rsvd = sizeof(heap_t);
    
    for (i = KHEAP_START; i < KHEAP_INIT_END; i+=0x1000)
        get_page(i, 1, kernel_dir);

    i = 0;
    while (i < placement_addr + kheap_rsvd) {
        page = get_page(i, 1, kernel_dir);

        if (!page->addr) {
            alloc_frame(page, 1, 0);
        }
        
        i += 0x1000;
    }

    page = get_page(0, 0, kernel_dir);
    memset(page, 0, sizeof(page_t));

    for (i = KHEAP_START; i < KHEAP_INIT_END; i+=0x1000)
        alloc_frame(get_page(i, 0, kernel_dir), 0, 0);

    register_interrupt_handler(14, &page_fault_handler);

    switch_page_directory(kernel_dir);

//    asm volatile("xchg %bx,%bx");

    scr_puts("create_heap\n");
    kheap = create_heap(KHEAP_START, KHEAP_INIT_END, KHEAP_MAX_END, 0, 0);

    current_dir = clone_directory(kernel_dir);

    switch_page_directory(current_dir);

//    asm volatile("xchg %bx,%bx");
}

void switch_page_directory(page_directory_t *dir)
{
//    scr_putp("switch_page_directory to", dir->addr);
    current_dir = dir;
    asm volatile ( "movl %0, %%cr3" : : "r"(dir->addr));
    u32int cr0;
    asm volatile ( "movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ( "movl %0, %%cr0" : : "r"(cr0));
}

page_t *get_page(u32int vaddr, u32int make, page_directory_t *dir)
{
    if (!dir) 
        return 0;

    vaddr = vaddr/0x1000;

    u32int table_idx = vaddr / 1024;
    u32int page_idx = vaddr % 1024;

    if (dir->page_tables[table_idx]) {
        return &dir->page_tables[table_idx]->pages[page_idx];
    } else if (make) {
        /*scr_putp("make page table for addr", vaddr*0x1000);*/
        u32int tmp;
        dir->page_tables[table_idx] = (page_table_t *)kmalloc_ap(sizeof(page_table_t),&tmp);
        memset(dir->page_tables[table_idx], 0, sizeof(page_table_t));
        dir->table_addr[table_idx] = tmp | 0x07;  // PRESENT, RW, US

        return(&dir->page_tables[table_idx]->pages[page_idx]);
    } else {
        return 0;
    }
}

void copy_page(page_t *src, page_t *dst)
{
    ASSERT(src);
    ASSERT(src->addr);
    ASSERT(dst);
    ASSERT(dst->addr);

    copy_physical_frame(src->addr*0x1000, dst->addr*0x1000);
}

void page_fault_handler(registers_t *regs)
{
    u32int fault_addr;
    asm volatile("movl %%cr2, %0" : "=r"(fault_addr));

   // The error code gives us details of what happened.
    u32int unpresent   = !(regs->errcode & 0x1); // Page not present
    u32int rw = regs->errcode & 0x2;           // Write operation?
    u32int us = regs->errcode & 0x4;           // Processor was in user-mode?
    //u32int reserved = regs->errcode & 0x8;     // Overwritten CPU-reserved bits of page entry?
    u32int id = regs->errcode & 0x10;          // Caused by an instruction fetch?    
    printk("Page fault %p (%c%c%c),eip %p", fault_addr, unpresent?'P':'p',
            rw?'W':'w', us?'U':'u', regs->eip);

    if (!current_task) {
        PANIC("\nPage fault in mono task env.");
    }

    if (us) {
        if (handle_user_page_fault(fault_addr,rw) != PFAULT_HANDLED) {
            printk(" (FAIL)\n");
            exit(EFAULT);
        } else {
            printk(" (PASS)\n");
        }
    } else {
        PANIC("Page fault: kernel oops.");
    }

}

void alloc_frame(page_t *page, u32int is_user, u32int is_writable)
{
    if (page->addr) {
//        scr_putp("page->addr non null", page->addr);
        return;
    }
    u32int frame = first_frame();
    if (frame == 0xffffffff)
        PANIC("alloc frame failed:not enough memory!\n");
    else {
        set_frame(frame * 0x1000);
        page->present = 1;
        page->rw = is_writable?1:0;
        page->us = is_user?1:0;
        page->addr = frame;
    }
}

u32int test_frame(u32int frame_addr)
{
    u32int frame = frame_addr / 0x1000;
    u32int idx = IDX_FROM_BITS(frame);
    u32int off = OFF_FROM_BITS(frame);
    return frames[idx] & (0x1 << off);
}

void set_frame(u32int frame_addr)
{
    u32int frame = frame_addr / 0x1000;
    u32int idx = IDX_FROM_BITS(frame);
    u32int off = OFF_FROM_BITS(frame);
    frames[idx] |= 0x1 << off;
}

void clear_frame(u32int frame_addr)
{
    u32int frame = frame_addr / 0x1000;
    u32int idx = IDX_FROM_BITS(frame);
    u32int off = OFF_FROM_BITS(frame);
    frames[idx] &= ~(0x1 << off);
}

u32int first_frame()
{
    u32int i, j;
    for (i = 0; i < IDX_FROM_BITS(nframes); i++) {
        if (frames[i] != 0xffffffff) {
            for (j = 0; j < 32; j++) {
                u32int to_test = 0x1 << j;
                if ( !(frames[i] & to_test) ) {
                    return i * 32 + j;
                }
            }
        }
    }
    return 0xffffffff;
}

void free_page(page_t *page)
{
    if (page) {
        clear_frame(page->addr * 0x1000);
        memset(page, 0, sizeof(page_t));
    }
}

page_table_t *clone_table(page_table_t *table, u32int *phys)
{
//    scr_puts("clone_table\n");
    page_table_t *new_table = (page_table_t *) kmalloc_ap(sizeof(page_table_t), phys);
    memset(new_table, 0, sizeof(page_table_t));

    u32int i = 0;

    for (i=0; i<1024; i++) {
        if (table->pages[i].addr) {
            alloc_frame(&new_table->pages[i], table->pages[i].rw, table->pages[i].us);
            new_table->pages[i].present = table->pages[i].present;
            new_table->pages[i].accessed = table->pages[i].accessed;
            new_table->pages[i].dirty = table->pages[i].dirty;

            copy_physical_frame(table->pages[i].addr*0x1000, new_table->pages[i].addr*0x1000);
        }
    }

    return new_table;

}

page_directory_t *clone_directory(page_directory_t *dir)
{
    // paging not inited
    if (kernel_dir == 0)
        return 0;

    u32int phys;
    u32int pt_phys;
    u32int i;

    page_directory_t *new_dir = (page_directory_t *)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(new_dir,0,sizeof(page_directory_t));

    for (i=0; i<1024; i++) {
        if (!dir->page_tables[i])
            continue;
        if (dir->page_tables[i] == kernel_dir->page_tables[i]) {
            new_dir->page_tables[i] = dir->page_tables[i];
            new_dir->table_addr[i] = dir->table_addr[i];
        } else {
            new_dir->page_tables[i] = clone_table(dir->page_tables[i], &pt_phys);
            new_dir->table_addr[i] = pt_phys | 0x07; // USER, WRITABLE
        }
    }

    u32int offset = (u32int)new_dir->table_addr - (u32int)new_dir;
    new_dir->addr = phys + offset;

    /*printk("clone dir %p --> new_dir %p\n", dir->addr, new_dir->addr);*/
    return new_dir;
}

