/*
 * =============================================================================
 *
 *       Filename:  vmm.c
 *
 *    Description:  Manage Virtual Memory Areas
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 23时46分07秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#include "vmm.h"
#include "kmm.h"
#include "util.h"

struct VMA *vmaCreate(u64int start, u64int size, u64int flags)
{
    struct VMA *vma;

    vma = (struct VMA*)kMalloc(sizeof(struct VMA));
    memset(vma,0,sizeof(struct VMA));
    vma->start = start;
    vma->size = size;
    vma->flags = flags;
    return vma;
}

struct VM *vmCreate()
{
    struct VM *vm;
    struct VMA *vma, *tmp;
    u64int n;

    vm = (struct VM*)kMalloc(sizeof(struct VM));
    memset(vm,0,sizeof(struct VM));

    // reserved first 4KB
    vm->vmaHead = vmaCreate(0, PAGE_SIZE, VMA_STATUS_INVALID);
    tmp = vm->vmaHead;

    // 4KB-1MB kernel used
    vma = vmaCreate(PAGE_SIZE, HIGHMEM_START_ADDR - PAGE_SIZE, 
            VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_HEAP);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // 1MB-Kernel Heap Free
    vma = vmaCreate(HIGHMEM_START_ADDR, HEAP_START_ADDR - HIGHMEM_START_ADDR, VMA_STATUS_FREE);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // physical memory mapped
    vma = vmaCreate(HEAP_START_ADDR, totalMemory(), VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_HEAP);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // Free gap before kernel code
    n = CODE_LOAD_ADDR - tmp->start - tmp->size;
    vma = vmaCreate(tmp->start + tmp->size, n, VMA_STATUS_FREE);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // Kernel Code
    vma = vmaCreate(CODE_LOAD_ADDR, MIN(MAX_CODE_SIZE,totalMemory()), VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_HEAP);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;


    // free region until meet kernel stack
    n = KERNEL_STACK_TOP - KERNEL_STACK_SIZE - CODE_LOAD_ADDR - MIN(MAX_CODE_SIZE,totalMemory());
    vma = vmaCreate(tmp->start+tmp->size, n, VMA_STATUS_FREE);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // Kernel Stack
    vma = vmaCreate(tmp->start+tmp->size, KERNEL_STACK_SIZE, VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_STACK);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // free region in the end
    n = -KERNEL_STACK_TOP;
    vma = vmaCreate(tmp->start+tmp->size, n, VMA_STATUS_FREE);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;


//    vmDump(vm);
    return vm;
}

u64int vmAddArea(struct VM *vm, u64int start, u64int size, u64int flags)
{

}

struct VMA *vmQueryArea(struct VM *vm, u64int addr)
{
}

u64int vmRemoveArea(struct VM *vm,struct VMA *vma)
{

}

void vmaDump(struct VMA *vma)
{
    printk("VMA-Start:%x,Size:%x,Flags:%x\n",vma->start,vma->size,vma->flags);
}

void vmDump(struct VM *vm)
{
    struct VMA *vma;

    printk("VM--\n");
    vma = vm->vmaHead;
    while (vma) {
        vmaDump(vma);
        vma = vma->next;
    }
}

