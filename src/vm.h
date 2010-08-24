#ifndef VM_H
#define VM_H 

#include "common.h"
#include "paging.h"
#include "file.h"

#define VMA_ANON      1     /* memory mapping, no back-file*/
#define VMA_READ      2  
#define VMA_WRITE     4 
#define VMA_EXEC      8
#define VMA_STACK     16 
#define VMA_PRIVATE   32 
#define VMA_FILE      64 
#define VMA_SHARED    128

struct vm_struct;

/* 
 * represents a vma of the virtual memory space, eg, 0x20000000 ~ 0x30000000, 
 * which may refer to underline device, shared memory, normal memory.
 *
 */
typedef struct vma_struct {
    u32int start; /* page aligned start virtual address. */
    u32int end;
    u32int flags;

    struct vm_struct *vm;
    struct vma_struct *prev;
    struct vma_struct *next;

    void *priv; /* usually file_mapping_t* */
} vma_t;

// represents the user space task's virtual memory space
typedef struct vm_struct {
    u32int entry;      // task's entry point
    /*
     *u32int start_code; // .text usually 0x40000000;
     *u32int end_code;
     *u32int start_data; // .rodata .data .bss all combined in [start_data, end_data)
     *u32int end_data;
     */
    u32int start_brk;
    u32int brk;
    u32int start_stack; // task's userspace stack, usually 0xd0000000 start_stack > end_stack
    u32int end_stack;   // initially (0xd0000000 - 0x1000)
    u32int kernel_stack; // kernel_stack is in kernel_heap, size is 4kb, [kernel_stack-0x1000, kernel_stack)

    vma_t *vmas;

} vm_t;

// get vm vma contains start
vma_t* get_vma_cover(vm_t *vm, u32int start);

// get vm vma contains [start, end)
vma_t* get_vma_cover2(vm_t *vm, u32int start, u32int end);

// insert vm_vma into vm
void insert_vma(vm_t *vm, vma_t *vma);

// remove vm_vma from vm, doesn't free it
void remove_vma(vm_t *vm, vma_t *vma);

// remove all vm_vmas from vm, and kfree them
void free_vmas(vm_t *vm);

// return ret if ret>=start and [ret, ret+size) doesn't intersect with any vm_vma
u32int get_vm_free_start(vm_t *vm, u32int start, u32int size);

// called by page fault handler, vma lives in current_task->dir
void vma_nopage(vma_t *vma, u32int vaddr);

// vma must lives in current_task->dir
void vma_freepages(vma_t *vma);

// write to back-file
void vma_write(vma_t *vma, u32int vaddr, u32int size);

vma_t *clone_vma(vma_t *vma);

vm_t *clone_vm(vm_t *vm);

#endif /* VM_H */
