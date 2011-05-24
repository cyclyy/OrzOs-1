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

struct VMA *vmaCopy(struct VMA *oldVMA, struct PML4E *pml4e, u64int flags)
{
    struct VMA *newVMA;
    u64int i, vaddr, vaddr2, paddr, paddr2, deepCopy;

    newVMA = (struct VMA*)kMalloc(sizeof(struct VMA));
    memcpy(newVMA, oldVMA, sizeof(struct VMA));
    if (oldVMA->flags & VMA_STATUS_USED) {
        if (oldVMA->flags & VMA_OWNER_KERNEL) {
            if (oldVMA->flags & VMA_TYPE_STACK) 
                deepCopy = 1;
            else
                deepCopy = 0;
        } else {
            deepCopy = 1;
        }
        vaddr = newVMA->start;
        for (i=0; i<newVMA->size >> PAGE_LOG2_SIZE; i++) {
            if (deepCopy) {
                paddr2 = getPAddr(vaddr,getPML4E()); 
                vaddr2 = kMallocEx(PAGE_SIZE,1,&paddr);
                memcpy((void*)vaddr2, (void*)PADDR_TO_VADDR(paddr2), PAGE_SIZE);
            } else {
                paddr = getPAddr(vaddr,getPML4E()); 
            }
            mapPagesVtoP(vaddr, paddr, 1, pml4e);
            vaddr += PAGE_SIZE;
        }
    }

    return newVMA;
}

struct VM *vmCreate()
{
    struct VM *vm;
    struct VMA *vma, *tmp;
    u64int n;

    vm = (struct VM*)kMalloc(sizeof(struct VM));
    memset(vm,0,sizeof(struct VM));

    asm volatile("mov %%cr3,%0":"=r"(vm->cr3)::);
    vm->cr3 = vm->cr3 & 0xfffffffffffff000;

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

struct VM *vmCopy(struct VM *oldVM, u64int flags)
{
    struct VM *newVM;
    struct VMA *vma, *newVMA, *v;
    struct PML4E *pml4e;

    newVM = (struct VM *)kMalloc(sizeof(struct VM));
    memset(newVM, 0, sizeof(struct VM));
    pml4e = (struct PML4E *)kMallocEx(sizeof(struct PML4E),1,0);
    newVM->cr3 = VADDR_TO_PADDR(pml4e);
    memset(pml4e, 0, sizeof(struct PML4E));
    vma = oldVM->vmaHead;
    v = 0;
    while (vma) {
        newVMA = vmaCopy(vma, pml4e, flags);
        
        if (!v) 
            newVM->vmaHead = newVMA;
        else {
            newVMA->prev = v;
            v->next = newVMA;
        }
        v = newVMA; 
        vma = vma->next;
    }

    return newVM;
}

s64int vmDestroy(struct VM *vm)
{
    kFree(vm);
    return 0;
}

s64int vmAddArea(struct VM *vm, u64int start, u64int size, u64int flags)
{
    return 0;
}

struct VMA *vmQueryArea(struct VM *vm, u64int addr)
{
    return 0;
}

s64int vmRemoveArea(struct VM *vm,struct VMA *vma)
{
    return 0;
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

