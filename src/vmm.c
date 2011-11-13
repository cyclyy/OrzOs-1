#include "vmm.h"
#include "paging.h"
#include "kmm.h"
#include "util.h"

struct VM *kernelVM = 0;

static struct VMA *findFreeArea(struct VM *vm, u64int size, u64int startBound, 
        u64int endBound, u64int direct)
{
    struct VMA *v;

    if (((direct != VMA_DIR_UP) && (direct != VMA_DIR_DOWN)) 
            || (size > endBound - startBound))
        return 0;

    if (direct == VMA_DIR_UP) {
        v = vmQueryAreaByAddr(vm, startBound);
        while (v && (v->start < endBound)) {
            if ((v->flags == VMA_STATUS_FREE) && 
                    (MAX(startBound, v->start) + size <= MIN(endBound, v->start+v->size)))
                return v;
            v = v->next;
        }
        return 0;
    } else {
        v = vmQueryAreaByAddr(vm, endBound);
        while (v && (v->start + v->size > startBound)) {
            if ((v->flags == VMA_STATUS_FREE) && 
                    (MAX(startBound, v->start) + size <= MIN(endBound, v->start+v->size)))
                return v;
            v = v->prev;
        }
        return 0;
    }
}

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
    struct PML4E *oldPML4E;
    struct PageDirectoryPointer *pdp, *oldPDP;
    struct PageDirectory *pd, *oldPD;
    struct PageTable *pt, *oldPT;
    u64int *page, *oldPage;
    u64int i, vaddr, vaddr2, paddr, paddr2, deepCopy, endAddr;
    u64int pml4eIdx, pdpIdx, pdIdx, ptIdx, physAddr;

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
        if (deepCopy) {
            for (i=0; i<newVMA->size >> PAGE_LOG2_SIZE; i++) {
                paddr2 = getPAddr(vaddr,getPML4E()); 
                vaddr2 = kMallocEx(PAGE_SIZE,1,&paddr);
                memcpy((void*)vaddr2, (void*)PADDR_TO_VADDR(paddr2), PAGE_SIZE);
                mapPagesVtoP(vaddr, paddr, 1, pml4e, oldVMA->flags & VMA_OWNER_USER);
                vaddr += PAGE_SIZE;
            }
        } else {
            endAddr = newVMA->start + newVMA->size;
            oldPML4E = getPML4E();
            while (vaddr < endAddr) {
                parseVAddr(vaddr, &pml4eIdx, &pdpIdx, &pdIdx, &ptIdx);
                pdp = getPDP(pml4e, vaddr);
                oldPDP = getPDP(oldPML4E, vaddr);
                if (!pdp) {
                    if (!pdpIdx && !pdIdx && !ptIdx && (((u64int)1<<39) <= endAddr-vaddr)) {
                        pml4e->pdp[pml4eIdx] = VADDR_TO_PADDR(oldPDP) | 7;
                        vaddr += (u64int)1<<39;
                        continue;
                    }
                    pdp = (struct PageDirectoryPointer *)kMallocEx(4096,1,&physAddr);
                    memset(pdp, 0, sizeof(struct PageDirectoryPointer));
                    pml4e->pdp[pml4eIdx] = physAddr | 7;
                }
                pd = getPD(pdp, vaddr);
                oldPD = getPD(oldPDP, vaddr);
                if (!pd) {
                    if (!pdIdx && !ptIdx && ((1<<30) <= endAddr-vaddr)) {
                        pdp->pd[pdpIdx] = VADDR_TO_PADDR(oldPD) | 7;
                        vaddr += 1<<30;
                        continue;
                    }
                    pd = (struct PageDirectory *)kMallocEx(4096,1,&physAddr);
                    memset(pd, 0, sizeof(struct PageDirectory));
                    pdp->pd[pdpIdx] = physAddr | 7;
                }
                pt = getPT(pd, vaddr);
                oldPT = getPT(oldPD, vaddr);
                if (!pt) {
                    if (!ptIdx && ((1<<21) <= endAddr-vaddr)) {
                        pd->pt[pdIdx] = VADDR_TO_PADDR(oldPT) | 7;
                        vaddr += 1<<21;
                        continue;
                    }
                    pt = (struct PageTable *)kMallocEx(4096,1, &physAddr);
                    memset(pt, 0, sizeof(struct PageTable));
                    pd->pt[pdIdx] = physAddr | 7;
                }
                page = getPage(pt, vaddr);
                oldPage = getPage(oldPT, vaddr);
                *page = *oldPage | 3;
                if (flags & VMA_OWNER_USER) 
                    *page |= 4;
                vaddr += PAGE_SIZE;
            }
        }
    }

    return newVMA;
}

struct VM *vmCreate()
{
    return vmCopy(kernelVM, 0);
}

struct VM *vmInit()
{
    struct VM *vm;
    struct VMA *vma, *tmp;
    u64int n;

    if (kernelVM)
        return kernelVM;

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
    vma = vmaCreate(HIGHMEM_START_ADDR, KERNEL_HEAP_START_ADDR - HIGHMEM_START_ADDR, VMA_STATUS_FREE);
    vma->prev = tmp;
    tmp->next = vma;
    tmp = vma;

    // physical memory mapped
    vma = vmaCreate(KERNEL_HEAP_START_ADDR, totalMemory(), VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_HEAP);
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
    vmAddArea(vm,MMAP_DEV_START_ADDR,MMAP_DEV_SIZE,VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_HEAP);
    kernelVM = vm;

    vmRef(vm);

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

    vmRef(newVM);

    return newVM;
}

s64int vmTruncate(struct VM *vm)
{
    return 0;
}

s64int vmDestroy(struct VM *vm)
{
    struct VMA *v, *pv;
    struct PML4E *pml4e, *kPML4E;
    struct PageDirectoryPointer *pdp, *kPDP;
    struct PageDirectory *pd, *kPD;
    struct PageTable *pt, *kPT;
    u64int i,j,k;
    v = vm->vmaHead;
    pv = v;
    while (v) {
        vmRemoveArea(vm, v);
        pv = v;
        v = v->next;
    }
    pml4e = (struct PML4E*)PADDR_TO_VADDR(vm->cr3);
    kPML4E = (struct PML4E*)PADDR_TO_VADDR(kernelVM->cr3);
    for (i=0; i<512; i++) {
        if (!pml4e->pdp[i])
            continue;
        pdp = (struct PageDirectoryPointer*)PADDR_TO_VADDR(pml4e->pdp[i] & PAGE_MASK);
        kPDP = (struct PageDirectoryPointer*)PADDR_TO_VADDR(kPML4E->pdp[i] & PAGE_MASK);
        for (j=0; j<512; j++) {
            if (!pdp->pd[j])
                continue;
            pd = (struct PageDirectory*)PADDR_TO_VADDR(pdp->pd[j] & PAGE_MASK);
            if (kPDP)
                kPD = (struct PageDirectory*)PADDR_TO_VADDR(kPDP->pd[j] & PAGE_MASK);
            else 
                kPD = 0;
            for (k=0; k<512; k++) {
                if (!pd->pt[k])
                    continue;
                pt = (struct PageTable*)PADDR_TO_VADDR(pd->pt[k] & PAGE_MASK);
                if (kPD)
                    kPT = (struct PageTable*)PADDR_TO_VADDR(kPD->pt[k] & PAGE_MASK);
                else
                    kPT = 0;
                if (pt != kPT) {
                    DBG("Free pt %x",buildVAddr(i,j,k,0));
                    kFree(pt);
                    pd->pt[k] = 0;
                }
            }
            if (pd != kPD) {
                DBG("Free pd %x",buildVAddr(i,j,0,0));
                kFree(pd);
                pdp->pd[j] = 0;
            }
        }
        if (pdp != kPDP) {
            DBG("Free pdp %x",buildVAddr(i,0,0,0));
            kFree(pdp);
            pml4e->pdp[i] = 0;
        }
    }
    DBG("Free pml4e");
    kFree(pml4e);
    kFree(pv);
    kFree(vm);
    return 0;
}

s64int vmAddArea(struct VM *vm, u64int start, u64int size, u64int flags)
{
    struct VMA *vma, *v;
    struct PML4E *pml4e;
    u64int vaddr, paddr, i;

    pml4e = (struct PML4E *)PADDR_TO_VADDR(vm->cr3);

    v = vmQueryAreaByAddr(vm, start);
    if ((v->flags & VMA_STATUS_FREE) && (v->start+v->size>=start+size)) {
        if (start > v->start) {
            vma = vmaCreate(v->start, start - v->start, VMA_STATUS_FREE);
            vma->next = v;
            vma->prev = v->prev;
            if (v->prev)
                v->prev->next = vma;
            v->prev = vma;
        }
        if (v->start+v->size>start+size) {
            vma = vmaCreate(start+size, v->start + v->size - start - size, VMA_STATUS_FREE);
            vma->next = v->next;
            vma->prev = v;
            if (v->next)
                v->next->prev = vma;
            v->next = vma;
        }
        v->start = start;
        v->size = size;
        v->flags = flags;

        if (flags & VMA_STATUS_USED) {
            if ((flags & VMA_OWNER_USER) || 
                    ((flags & VMA_OWNER_KERNEL) && (flags & VMA_TYPE_STACK))) {

                for (i=0; i<size; i+=PAGE_SIZE) {
                    vaddr = start + i;
                    kMallocEx(PAGE_SIZE, 1, &paddr);
                    if (paddr) 
                        mapPagesVtoP(vaddr, paddr, 1, pml4e, flags & VMA_OWNER_USER);
                    else {
                        return -1;
                    }
                }
            }
        }

        return 0;
    } else {
        return -1;
    }
}

s64int vmMapArea(struct VM *vm, u64int start, u64int size, u64int flags, u64int paddr)
{
    struct PML4E *pml4e;
    pml4e = (struct PML4E *)PADDR_TO_VADDR(vm->cr3);
    if ((flags & VMA_STATUS_USED) && (flags & VMA_TYPE_MMAP)) {
        if (start == 0) {
            start = vmAllocArea(vm, 0, size, USER_HEAP_START_ADDR, USER_HEAP_END_ADDR, VMA_DIR_DOWN, flags);
        } else {
            vmAddArea(vm, start, size, flags);
        }
        mapPagesVtoP(start, paddr, size / PAGE_SIZE, pml4e, flags & VMA_OWNER_USER);
    }
    return start;
}

s64int vmUnmapArea(struct VM *vm, u64int start, u64int size)
{
    struct VMA *v;

    v = vmQueryAreaByAddr(vm, start);

    if (v && (v->start == start) && (v->size == size)) {
        vmRemoveArea(vm, v);
        return 0;
    }
    return -1;
}

u64int vmAllocArea(struct VM *vm, u64int startHint, u64int size, 
        u64int startBound, u64int endBound, u64int direct, u64int flags)
{
    struct VMA *v;
    u64int retAddr;

    if ((direct != VMA_DIR_UP) && (direct != VMA_DIR_DOWN))
        return 0;

    flags |= VMA_STATUS_USED;
    if (startHint) {
        if ((startHint >= startBound) && (startHint + size <= endBound)) {
            if (vmAddArea(vm, startHint, size, flags)==0) {
                v = vmQueryAreaByAddr(vm, startHint);
                return v->start;
            } else 
                return 0;
        } else {
            return 0;
        }
    }
    v = findFreeArea(vm, size, startBound, endBound, direct);
    if (!v)
        return 0;
    if (v->size == size) {
        v->flags = flags;
        return v->start;
    } else if (direct==VMA_DIR_UP) {
        retAddr = MAX(startBound,v->start);
        vmAddArea(vm,retAddr,size,flags);
        return retAddr;
    } else {
        retAddr = MIN(endBound-size,v->start+(v->size-size));
        vmAddArea(vm,retAddr,size,flags);
        return retAddr;
    }
}

struct VMA *vmQueryAreaByAddr(struct VM *vm, u64int addr)
{
    struct VMA *v;
    v = vm->vmaHead;
    while (v) {
        if ((addr >= v->start) && (addr < v->start+v->size))
            return v;
        v = v->next;
    }
    return 0;
}

s64int vmRemoveArea(struct VM *vm,struct VMA *vma)
{
    struct VMA *v;
    struct PML4E *pml4e;
    u64int i;

    if (vma->flags & VMA_STATUS_FREE) 
        return -1;
    pml4e = (struct PML4E *)PADDR_TO_VADDR(vm->cr3);
    if (vma->flags & VMA_STATUS_USED) {
        for (i=vma->start; i<vma->start+vma->size; i+=PAGE_SIZE) {
            if ((vma->flags & VMA_OWNER_USER) || 
                    ((vma->flags & VMA_OWNER_KERNEL) && (vma->flags & VMA_TYPE_STACK))) {
                if (!(vma->flags & VMA_TYPE_MMAP)) {
                    kFree((void*)PADDR_TO_VADDR(getPAddr(i,pml4e)));
                }
                unmapPages(i, 1, pml4e);
            }
        }
    }
    vma->flags = VMA_STATUS_FREE;
    v = vma->prev;
    if (v && (v->flags & VMA_STATUS_FREE)) {
        if (v->prev) 
            v->prev->next = v->next;
        if (v->next)
            v->next->prev = v->prev;
        vma->start = v->start;
        vma->size += v->size;
        vma->prev = v->prev;
        if (vm->vmaHead == v)
            vm->vmaHead = vma;
        kFree(v);
    }
    v = vma->next;
    if (v && (v->flags & VMA_STATUS_FREE)) {
        if (v->prev) 
            v->prev->next = v->next;
        if (v->next)
            v->next->prev = v->prev;
        vma->size += v->size;
        vma->next = v->next;
        kFree(v);
    }
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

/*
   s64int vmemcpy(struct VM *destVM, void *dest, struct VM *srcVM, void *src, u64int size)
   {
   u64int daddr, saddr;
   u64int i, remain;
   struct PML4E *destPML4E, *srcPML4E;

   destPML4E = (struct PML4E*)PADDR_TO_VADDR(destVM->cr3);
   srcPML4E = (struct PML4E*)PADDR_TO_VADDR(srcVM->cr3);

   remain = size & 0xfff;
   size -= remain;
   for (i=0; i<size; i+=PAGE_SIZE) {
   daddr = PADDR_TO_VADDR(getPAddr((u64int)dest+i, destPML4E));
   saddr = PADDR_TO_VADDR(getPAddr((u64int)src+i, srcPML4E));
//DBG("daddr:%x,saddr:%x",daddr,saddr);
memcpy((void*)daddr, (void*)saddr, PAGE_SIZE);
}
if (remain) {
daddr = PADDR_TO_VADDR(getPAddr((u64int)dest+size, destPML4E));
saddr = PADDR_TO_VADDR(getPAddr((u64int)src+size, srcPML4E));
//DBG("daddr:%x,saddr:%x",daddr,saddr);
memcpy((void*)daddr, (void*)saddr, remain);
}
return 0;
}
*/

u64int vmemcpy(struct VM *destVM, void *dest, struct VM *srcVM, void *src, u64int size)
{
    u64int remain, srcAddr, destAddr, srcSize, destSize, n, d1, d2;
    struct PML4E *destPML4E, *srcPML4E;

    destPML4E = (struct PML4E*)PADDR_TO_VADDR(destVM->cr3);
    srcPML4E = (struct PML4E*)PADDR_TO_VADDR(srcVM->cr3);

    srcAddr = (u64int)src;
    destAddr = (u64int)dest;
    remain = size;
    srcSize = PAGE_SIZE - srcAddr % PAGE_SIZE;
    destSize = PAGE_SIZE - destAddr % PAGE_SIZE;
    d1 = PADDR_TO_VADDR(getPAddr(srcAddr,srcPML4E));
    d2 = PADDR_TO_VADDR(getPAddr(destAddr,destPML4E));

    while (d1 && d2 && remain) {
        n = MIN( MIN(srcSize, destSize), remain);
        memcpy((void*)d2,(void*)d1,n);
        remain -= n;
        srcSize -= n;
        destSize -= n;
        srcAddr += n;
        destAddr += n;
        if (srcSize == 0) {
            d1 = PADDR_TO_VADDR(getPAddr(srcAddr,srcPML4E));
            srcSize = PAGE_SIZE;
        } else {
            d1 += n;
        }
        if (destSize == 0) {
            d2 = PADDR_TO_VADDR(getPAddr(destAddr,destPML4E));
            destSize = PAGE_SIZE;
        } else {
            d2 += n;
        }
    }

    return size - remain;
}

struct VM *vmRef(struct VM *vm)
{
    vm->ref++;
    return vm;
}

s64int vmDeref(struct VM *vm)
{
    if (vm->ref<=0)
        return -1;

    vm->ref--;

    if (vm->ref == 0)
        return vmDestroy(vm);

    return 0;
}

u64int copyFromUser(void *dest, void *src, u64int size)
{
    return copyUnsafe(dest,src,size);
}

u64int copyToUser(void *dest, void *src, u64int size)
{
    return copyUnsafe(dest,src,size);
}

