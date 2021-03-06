#include "kmm.h"
#include "paging.h"
#include "util.h"
#include "debugcon.h"
#include "tlsf.h"
#include "bootinfo.h"

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

static u64int analyseTotalMemory()
{
    struct BootInfo *bi;
    struct PhysicalMemoryChunk *chunk;
    u64int i, ret;
    bi = getBootInfo();
    ret = 0;
    i = 0;
    chunk = bi->chunks;
    while (i < bi->mmapSize) {
        if ((chunk->type == PMEM_CHUNK_FREE) && (chunk->start == HIGHMEM_START_ADDR)) {
            ret = chunk->size;
            break;
        }
        i += chunk->recLen + 4;
        chunk = (struct PhysicalMemoryChunk*)(((u64int)bi->chunks) + i);
    }
    return ret;
}

void initMemoryManagement(u64int freePMemStartAddr)
{ 
    u64int n;
    u64int availableMemory;
    struct BootInfo *bi;

    bi = getBootInfo();
    // convert KB to bytes
    totalHighMemory = analyseTotalMemory();
    if (!totalHighMemory) {
        PANIC("InitMemoryFailed, unable to detect high memory");
    }
    /*
    if (totalHighMemory > 16*1024*1024)
        totalHighMemory = 16*1024*1024;
        */
    freeMemoryStart = CODE_LOAD_ADDR + ((bi->freeMemStartAddr + PAGE_SIZE - 1) & (~0 << 12));

    n = (totalHighMemory+HIGHMEM_START_ADDR) >> 12;
    //printk("start mapPages:%x\n",n);
    mapPagesVtoP(CODE_LOAD_ADDR,0,MIN(MAX_CODE_SIZE>>12,n),getPML4E(), 0);
    mapPagesVtoP(MMAP_DEV_START_ADDR,MMAP_DEV_START_ADDR,MMAP_DEV_SIZE >> 12,getPML4E(), 0);
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
                //DBG("%x,%x,%x,%x,%x,%x",blk,blk->size,getNextBlock(blk), *(u64int*)getNextBlock(blk), getNextBlock(getNextBlock(blk)), *(u64int*)getNextBlock(getNextBlock(blk)));
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
    struct TLSFBlock *blk;
    //DBG("Before: %x,%x,%x,%x",blk,*(u64int*)blk, nextBlk, *(u64int*)nextBlk);
    if (!addr)
        return;
    if (!heap) {
        printk("Trying to free memory while heap is not created.");
    } else {
        if (((u64int)addr > (u64int)heap->blockHead) && 
                ((u64int)addr < (u64int)heap->blockEnd)) {
            blk = *(struct TLSFBlock **)(addr - PTR_SIZE);
            if (((u64int)addr < (u64int)&blk->ptr.buffer) || ((u64int)addr >= (u64int)&blk->ptr.buffer + (blk->size & BLOCK_SIZE))) {
                PANIC("Free bad pointer %x", addr);
            }
            tlsfFree(heap,blk);
        } else
            PANIC("Free bad pointer %x", addr);
        //DBG(" After:%x,%x,%x,%x",blk,*(u64int*)blk,nextBlk, *(u64int*)nextBlk);
    }
}

