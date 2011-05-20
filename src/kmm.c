/*
 * =============================================================================
 *
 *       Filename:  kmm.c
 *
 *    Description:  Low level memory management
 *
 *        Version:  1.0
 *        Created:  2011年05月14日 11时25分25秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#include "kmm.h"
#include "util.h"
#include "screen.h"
#include "tlsf.h"

#define INVLPG(x) {asm volatile("invlpg %0"::"m"(*(char*)x)); }
#define MAP_ADDR_P_TO_V(x) ((x) ? (heap ? ((u64int)(x) + HEAP_START_ADDR):((u64int)(x) + CODE_LOAD_ADDR)) : 0)
#define MAP_ADDR_V_TO_P(x) ((x) ? (heap ? ((u64int)(x) - HEAP_START_ADDR):((u64int)(x) + CODE_LOAD_ADDR)) : 0)

/*-----------------------------------------------------------------------------
 *  they're all in bytes
 *-----------------------------------------------------------------------------*/
u64int totalHighMemory;                     
u64int freeMemoryStart;                     

struct TLSFHeader *heap = 0;
int memoryMapped = 0;

u64int totalMemory() 
{
    return totalHighMemory & (~0 - PAGE_SIZE + 1);
}

u64int availMemory() 
{
    if (!heap)
        return totalHighMemory - (freeMemoryStart - CODE_LOAD_ADDR);
    else 
        return heap->maxSize - heap->usedSize;
}

struct PML4E *getPML4E()
{
    u64int x;
    asm volatile("mov %%cr3,%0":"=r"(x)::);
    if (heap)
        x += HEAP_START_ADDR;
    return (struct PML4E *)(x & 0xfffffffffffff000);
}

struct PageDirectoryPointer *getPDP(struct PML4E *pml4e, u64int addr)
{
    return (struct PageDirectoryPointer *)
        MAP_ADDR_P_TO_V(pml4e->pdp[((addr>>39) & 511)] & (~0 - 0x1000 + 1));
}

struct PageDirectory *getPD(struct PageDirectoryPointer *pdp, u64int addr)
{

    return (struct PageDirectory *)
        MAP_ADDR_P_TO_V(pdp->pd[((addr>>30) & 511)] & (~0 - 0x1000 + 1));
}

struct PageTable *getPT(struct PageDirectory *pd, u64int addr)
{

    return (struct PageTable *)
        MAP_ADDR_P_TO_V(pd->pt[((addr>>21) & 511)] & (~0 - 0x1000 + 1));
}

u64int* getPage(struct PageTable *pt, u64int addr)
{
    return (u64int*)pt + ((addr>>12) & 511);
}


/*-----------------------------------------------------------------------------
 *  Map n pages start from vaddr to paddr
 *-----------------------------------------------------------------------------*/
void mapPagesVtoP(u64int vaddr, u64int paddr, u64int n, struct PML4E *pml4e)
{
    u64int i, physAddr;
    u64int *page;
    struct PageTable *pt;
    struct PageDirectory *pd;
    struct PageDirectoryPointer *pdp;

    for (i=0; i<n; i++) {
        pdp = getPDP(pml4e, vaddr);
        if (!pdp) {
            pdp = (struct PageDirectoryPointer *)kMallocEx(4096,1,&physAddr);
            memset(pdp, 0, sizeof(struct PageDirectoryPointer));
            pml4e->pdp[((vaddr>>39) & 511)] = physAddr | 3;
        }
        pd = getPD(pdp,  vaddr);
        if (!pd) {
            pd = (struct PageDirectory *)kMallocEx(4096,1,&physAddr);
            memset(pd, 0, sizeof(struct PageDirectory));
            pdp->pd[(vaddr>>30) & 511] = physAddr | 3;
        }
        pt = getPT(pd, vaddr);
        if (!pt) {
            pt = (struct PageTable *)kMallocEx(4096,1, &physAddr);
            memset(pt, 0, sizeof(struct PageTable));
            pd->pt[(vaddr>>21) & 511] = physAddr | 3;
        }
        page = getPage(pt, vaddr);
        *page = paddr | 3;
        INVLPG(vaddr);
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }

    printk("pt Alloced:%x\n",vaddr);
}

/*-----------------------------------------------------------------------------
 *  Unmap n pages start from vaddr
 *-----------------------------------------------------------------------------*/
void unmapPages(u64int vaddr, u64int n)
{
    u64int i;
    u64int *page;
    struct PageTable *pt;
    struct PageDirectory *pd;
    struct PageDirectoryPointer *pdp;
    struct PML4E *pml4e;

    pml4e = getPML4E();
    for (i=0; i<n; i++) {
        pdp = getPDP(pml4e, vaddr);
        if (!pdp)
            return;
        pd = getPD(pdp, vaddr);
        if (!pd)
            return;
        pt = getPT(pd, vaddr);
        if (!pt)
            return;
        page = getPage(pt, vaddr);
        *page = 0;
        INVLPG(vaddr);
        vaddr += PAGE_SIZE;
    }

}

void initMemoryManagement(u64int totalHighMem, u64int freePMemStartAddr)
{ 
    u64int n;
    u64int availableMemory;

    // convert KB to bytes
    totalHighMemory = totalHighMem*1024;
    freeMemoryStart = CODE_LOAD_ADDR + ((freePMemStartAddr + PAGE_SIZE - 1) & (~0 << 12));

    n = (totalHighMemory+HIGHMEM_START_ADDR) >> 12;
    printk("start mapPages:%x\n",n);
    mapPagesVtoP(CODE_LOAD_ADDR,0,MIN(MAX_CODE_SIZE>>12,n),getPML4E());
    mapPagesVtoP(HEAP_START_ADDR,0,n,getPML4E());

    availableMemory = totalHighMemory - (freeMemoryStart - CODE_LOAD_ADDR);
    freeMemoryStart = freeMemoryStart - CODE_LOAD_ADDR + HEAP_START_ADDR;
    availableMemory &= ~0 << 3;

    printk("start createHeap\n");
    heap = tlsfInitHeap(freeMemoryStart, availableMemory);

    printk("heap created\n");

    unmapPages(0, 1);
}

u64int kMallocEx(u64int size, u64int pageAligned, u64int *physicalAddr)
{
    u64int ret;
    struct TLSFBlock *blk;

    if (!size) {
        return 0;
    }


    if (!heap) {
        if (pageAligned) {
            freeMemoryStart = ROUND_PAGE_ALIGN(freeMemoryStart);
        }
        freeMemoryStart = ROUND_MEM_ALIGN(freeMemoryStart);
        ret = freeMemoryStart;
        freeMemoryStart += size;
        if (physicalAddr)
            *physicalAddr = ret - CODE_LOAD_ADDR;
    } else {
        if (!pageAligned) {
            size += PTR_SIZE;
            blk = tlsfAlloc(heap,size);
            if (!blk) 
                ret = 0;
            else {
                ret = (u64int)&blk->ptr.buffer + PTR_SIZE;
                *((u64int*)(ret-PTR_SIZE)) = (u64int)blk;
            }
        } else {
            size += PAGE_SIZE;
            blk = tlsfAlloc(heap,size);
            if (!blk)
                ret = 0;
            else {
                ret = (u64int)&blk->ptr.buffer + PTR_SIZE;
                ret = (ret + PAGE_SIZE - 1) & (~0 - PAGE_SIZE + 1);
                *((u64int*)(ret-PTR_SIZE)) = (u64int)blk;
            }
        }
        if (physicalAddr) {
            if (ret)
                *physicalAddr = ret - HEAP_START_ADDR;
            else
                *physicalAddr = 0;
        }
    }
    return ret;
}

u64int kMalloc(u64int size)
{
    return kMallocEx(size, 0, 0);
}

void kFree(void *addr)
{
    if (!addr)
        return;
    if (!heap) {
        printk("Trying to free memory while heap is not created.");
    } else {
        tlsfFree(heap,*(struct TLSFBlock **)(addr - PTR_SIZE));
    }
}

