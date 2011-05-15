/*
 * =============================================================================
 *
 *       Filename:  mm.c
 *
 *    Description:  Memory Management System
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

#include "mm.h"
#include "util.h"
#include "screen.h"
#include "tlsf.h"

/*-----------------------------------------------------------------------------
 *  they're all in bytes
 *-----------------------------------------------------------------------------*/
u64int totalHighMemory;                     
u64int freeMemoryStart;                     
u64int availableMemory;

struct TLSFHeader *heap = 0;
int memoryMapped = 0;

u64int totalMemory() 
{
    return totalHighMemory;
}

u64int availMemory() 
{
    return availableMemory;
}


/*-----------------------------------------------------------------------------
 *  Map n pages start from vaddr to paddr
 *-----------------------------------------------------------------------------*/
void mapPagesEx(u64int vaddr, u64int paddr, u64int n)
{
    int i;

    for (i=0; i<n; i++) {
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }

}


void initPaging()
{
    u64int vaddr, paddr, i, x;
    struct Page *page;
    struct PageTable *pt;
    struct PageDirectory *pd;
    struct PageDirectoryPointer *pdp;
    struct PML4E *pml4e;

    asm volatile("mov %%cr3,%0":"=r"(x)::);
    pml4e = (struct PML4E *)(x & 0xfffffffffffff000);
    for (i = 0; i<totalHighMemory+HIGHMEM_START_ADDR; 
            i+=PAGE_SIZE) {
        paddr = i;
        vaddr = i + HEAP_START_ADDR;
        pdp = (struct PageDirectoryPointer *)
            (pml4e->pdp[((vaddr>>39) & 511)] & (~0 - 0x1000 + 1));
        if (!pdp) {
            pdp = (struct PageDirectoryPointer *)kmallocEx(4096,1,0);
            memset(pdp, 0, sizeof(struct PageDirectoryPointer));
            pml4e->pdp[((vaddr>>39) & 511)] = (u64int)pdp | 3;
        }
        pd = (struct PageDirectory *)
            (pdp->pd[(vaddr>>30) & 511] & (~0 - 0x1000 + 1));
        if (!pd) {
            pd = (struct PageDirectory *)kmallocEx(4096,1,0);
            memset(pd, 0, sizeof(struct PageDirectory));
            pdp->pd[(vaddr>>30) & 511] = (u64int)pd | 3;
        }
        pt = (struct PageTable *)
            (pd->pt[(vaddr>>21) & 511] & (~0 - 0x1000 + 1));
        if (!pt) {
            pt = (struct PageTable *)kmallocEx(4096,1,0);
            memset(pt, 0, sizeof(struct PageTable));
            pd->pt[(vaddr>>21) & 511] = (u64int)pt | 3;
        }
        pt->page[(vaddr>>12) & 511] = paddr | 3;
    }
}

void initMemoryManagement(u64int totalHighMem, u64int freePMemStartAddr)
{ 
    // convert KB to bytes
    totalHighMemory = totalHighMem*1024;
    freeMemoryStart = (freePMemStartAddr + PAGE_SIZE - 1) & (~0 << 12);

    initPaging();

    availableMemory = totalHighMemory - freeMemoryStart;
    freeMemoryStart += HEAP_START_ADDR;
    availableMemory &= ~0 << 3;

    heap = tlsfInitHeap(freeMemoryStart, availableMemory);

    u64int addr;
    kmallocEx(1,0,0);

    tlsfDump(heap);


}

u64int kmallocEx(u64int size, u64int pageAligned, u64int *physicalAddr)
{
    u64int ret;
    struct TLSFBlock *blk;

    if (!size) {
        return;
    }


    if (!heap) {
        if (pageAligned) {
            freeMemoryStart = ROUND_PAGE_ALIGN(freeMemoryStart);
        }
        freeMemoryStart = ROUND_MEM_ALIGN(freeMemoryStart);
        ret = freeMemoryStart;
        freeMemoryStart += size;
        if (physicalAddr)
            *physicalAddr = ret;
    } else {
        if (!pageAligned) {
            size += PTR_SIZE;
            blk = tlsfAlloc(heap,size);
            ret = (u64int)&blk->ptr.buffer + PTR_SIZE;
            *((u64int*)(ret-PTR_SIZE)) = (u64int)blk;
        } else {
            size += PAGE_SIZE;
            blk = tlsfAlloc(heap,size);
            ret = (u64int)&blk->ptr.buffer + PTR_SIZE;
            ret = (ret + PAGE_SIZE - 1) & (~0 - PAGE_SIZE + 1);
            *((u64int*)(ret-PTR_SIZE)) = (u64int)blk;

        }
        if (physicalAddr)
            *physicalAddr = ret - HEAP_START_ADDR;
    }
    return ret;
}

void free(u32int addr)
{
    if (!addr)
        return;
    if (!heap) {
        printk("Trying to free memory while heap is not created.");
    } else {
        tlsfFree(heap,*(struct TLSFBlock **)(addr - PTR_SIZE));
    }
}

