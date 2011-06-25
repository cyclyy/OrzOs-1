#include "kmm.h"
#include "paging.h"
#include "util.h"
#include "screen.h"
#include "tlsf.h"

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

void initMemoryManagement(u64int totalHighMem, u64int freePMemStartAddr)
{ 
    u64int n;
    u64int availableMemory;

    // convert KB to bytes
    totalHighMemory = totalHighMem*1024;
    if (totalHighMemory > 16*1024*1024)
        totalHighMemory = 16*1024*1024;
    freeMemoryStart = CODE_LOAD_ADDR + ((freePMemStartAddr + PAGE_SIZE - 1) & (~0 << 12));

    n = (totalHighMemory+HIGHMEM_START_ADDR) >> 12;
    //printk("start mapPages:%x\n",n);
    mapPagesVtoP(CODE_LOAD_ADDR,0,MIN(MAX_CODE_SIZE>>12,n),getPML4E(), 0);
    mapPagesVtoP(KERNEL_HEAP_START_ADDR,0,n,getPML4E(), 0);

    availableMemory = totalHighMemory - (freeMemoryStart - CODE_LOAD_ADDR);
    freeMemoryStart = freeMemoryStart - CODE_LOAD_ADDR + KERNEL_HEAP_START_ADDR;
    availableMemory &= ~0 << 3;

    //printk("start createHeap\n");
    heap = tlsfInitHeap(freeMemoryStart, availableMemory);

    //printk("heap created\n");

    unmapPages(0, 1, getPML4E());
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
                *physicalAddr = ret - KERNEL_HEAP_START_ADDR;
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
    //DBG("%x,%x",addr,*(struct TLSFBlock **)(addr - PTR_SIZE));
    if (!addr)
        return;
    if (!heap) {
        printk("Trying to free memory while heap is not created.");
    } else {
        if (((u64int)addr > (u64int)heap->blockHead) && 
                ((u64int)addr < (u64int)heap->blockEnd))
            tlsfFree(heap,*(struct TLSFBlock **)(addr - PTR_SIZE));
        else
            PANIC("Free bad pointer %x", addr);
    }
}

