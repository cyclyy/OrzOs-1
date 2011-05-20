/*
 * =============================================================================
 *
 *       Filename:  tlsf.c
 *
 *    Description:  Implementation of TLSF algorithm
 *
 *        Version:  1.0
 *        Created:  2011年05月15日 15时46分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#include "tlsf.h"
#include "kmm.h"
#include "util.h"

u64int msBit(u64int x)
{
    return 63 - __builtin_clzl(x);
}

u64int lsBit(u64int x)
{
    return __builtin_ffs(x) - 1;
}

void clearBit(u64int *x, u64int i)
{
    x[i>>6] &= ~(1 << (i & 0x3f));
}

void setBit(u64int *x, u64int i)
{
    x[i>>6] |= 1 << (i & 0x3f);
}

void tlsfAllocMapping(u64int *size, u64int *fl, u64int *sl)
{
    u64int t;
    if (*size < SMALL_BLOCK) {
        *fl = 0;
        *sl = *size / (SMALL_BLOCK / MAX_SLI);
    } else {
        t = (1 << (msBit(*size) - MAX_SLI_LOG2)) - 1;
        *size += t;
        *fl = msBit(*size);
        *size &= ~0 << (*fl - MAX_SLI_LOG2); 
        *sl = (*size >> (*fl - MAX_SLI_LOG2)) - MAX_SLI;
        *fl -= FLI_OFFSET;
    }
}

void tlsfMapping(u64int size, u64int *fl, u64int *sl)
{
    if (size < SMALL_BLOCK) {
        *fl = 0;
        *sl = size / (SMALL_BLOCK / MAX_SLI);
    } else {
        *fl = msBit(size);
        *sl = (size >> (*fl - MAX_SLI_LOG2)) - MAX_SLI;
        *fl -= FLI_OFFSET;
    }
}

struct TLSFBlock *tlsfFindFreeBlock(struct TLSFHeader *tlsf, u64int *fl, u64int *sl)
{
    struct TLSFBlock *blk = 0;
    u64int b1;

    b1 = tlsf->slBitmap[*fl] & (~0 << *sl);
    if (b1) {
        *sl = lsBit(b1);
        blk = tlsf->matrix[*fl][*sl];
    } else {
        *fl = lsBit(tlsf->flBitmap & (~0 << (*fl+1)));
        if (*fl != ~0) {
            *sl = lsBit(tlsf->slBitmap[*fl]);
            blk = tlsf->matrix[*fl][*sl];
        }
    }

    return blk;
}

void removeFreeBlock(struct TLSFBlock *blk, struct TLSFHeader *tlsf, u64int fl, u64int sl)
{
    if (blk->ptr.free.next)
        blk->ptr.free.next->ptr.free.prev = blk->ptr.free.prev;
    if (blk->ptr.free.prev)
        blk->ptr.free.prev->ptr.free.next = blk->ptr.free.next;
    if (tlsf->matrix[fl][sl] == blk) {
        tlsf->matrix[fl][sl] = blk->ptr.free.next;
        if (tlsf->matrix[fl][sl])
            tlsf->matrix[fl][sl]->ptr.free.prev = 0;
        else {
            clearBit(&tlsf->slBitmap[fl], sl);
            if (!tlsf->slBitmap[fl])
                clearBit(&tlsf->flBitmap, fl);
        }
    }
    tlsf->usedSize += blk->size & BLOCK_SIZE;
}

void insertFreeBlock(struct TLSFBlock *blk, struct TLSFHeader *tlsf, u64int fl, u64int sl)
{
    if (tlsf->matrix[fl][sl]) {
        tlsf->matrix[fl][sl]->ptr.free.prev = blk;
    }
    blk->ptr.free.prev = 0;
    blk->ptr.free.next = tlsf->matrix[fl][sl];
    tlsf->matrix[fl][sl] = blk;
    setBit(&tlsf->slBitmap[fl],sl);
    setBit(&tlsf->flBitmap,fl);
    tlsf->usedSize -= blk->size & BLOCK_SIZE;
}

struct TLSFBlock *getNextBlock(struct TLSFBlock *blk)
{
    return (struct TLSFBlock *)((u64int)blk + BLOCK_OVERHEAD + (blk->size & BLOCK_SIZE));
}

struct TLSFBlock *tlsfAlloc(struct TLSFHeader *tlsf, u64int size)
{
    u64int fl, sl, remainSize;
    struct TLSFBlock *freeBlock, *remainBlock, *nextBlock;

    if (size < MIN_BLOCK_SIZE)
        size = MIN_BLOCK_SIZE;

    size = ROUND_MEM_ALIGN(size);

    tlsfAllocMapping(&size, &fl, &sl);
    freeBlock = tlsfFindFreeBlock(tlsf, &fl, &sl);

    if (!freeBlock)
        return 0;

    removeFreeBlock(freeBlock, tlsf, fl, sl);
    freeBlock->size &= ~BLOCK_FREE;

    if (freeBlock->size >= size + SPLIT_THREHOLD) {
        remainSize = freeBlock->size - size;
        nextBlock = getNextBlock(freeBlock);
        freeBlock->size = size & ~BLOCK_FREE;
        remainBlock = getNextBlock(freeBlock);
        remainBlock->signature = TLSF_SIGNATURE;
        remainBlock->size = (remainSize - BLOCK_OVERHEAD) | BLOCK_FREE;
        remainBlock->prev = freeBlock;
        nextBlock->prev = remainBlock;
        tlsfMapping(remainSize - BLOCK_OVERHEAD, &fl, &sl);
        insertFreeBlock(remainBlock, tlsf, fl, sl);
    }

    return freeBlock;
}


struct TLSFHeader *tlsfInitHeap(u64int startAddr, u64int size)
{
    struct TLSFHeader *tlsf;
    struct TLSFBlock *prevBlock, *block, *nextBlock;

    tlsf = (struct TLSFHeader *)startAddr;
    memset(tlsf,0,sizeof(struct TLSFHeader));
    tlsf->maxSize = size;

    prevBlock = (struct TLSFBlock *)(startAddr + sizeof(struct TLSFHeader));
    memset(prevBlock,0,sizeof(struct TLSFBlock));
    prevBlock->signature = TLSF_SIGNATURE;
    prevBlock->size = MIN_BLOCK_SIZE;

    block = getNextBlock(prevBlock);
    memset(block,0,sizeof(struct TLSFBlock));
    block->signature = TLSF_SIGNATURE;
    block->size = size - sizeof(struct TLSFHeader) - BLOCK_OVERHEAD - (BLOCK_OVERHEAD + MIN_BLOCK_SIZE)*2;
    block->prev = prevBlock;

    nextBlock = getNextBlock(block);
    memset(nextBlock,0,sizeof(struct TLSFBlock));
    nextBlock->signature = TLSF_SIGNATURE;
    nextBlock->size = MIN_BLOCK_SIZE;
    nextBlock->prev = block;

    tlsf->blockHead = block;
    tlsf->blockEnd = nextBlock;

    tlsf->maxSize = block->size;
    tlsf->usedSize = tlsf->maxSize;

    tlsfFree(tlsf, block);

    return tlsf;
}

void tlsfFree(struct TLSFHeader *tlsf, struct TLSFBlock *blk)
{
    u64int fl, sl;
    struct TLSFBlock *prevBlock, *nextBlock;

    if (blk->signature != TLSF_SIGNATURE)
        printk("Free bad pointer\n");

    blk->ptr.free.prev = blk->ptr.free.next = 0;
    prevBlock = blk->prev;
    nextBlock = getNextBlock(blk);

    if (nextBlock->size & BLOCK_FREE) {
        tlsfMapping(nextBlock->size, &fl, &sl);
        removeFreeBlock(nextBlock,tlsf,fl,sl);
        blk->size += (nextBlock->size & BLOCK_SIZE) + BLOCK_OVERHEAD;
    }
    if (prevBlock->size & BLOCK_FREE) {
        tlsfMapping(prevBlock->size & BLOCK_SIZE, &fl, &sl);
        removeFreeBlock(prevBlock,tlsf,fl,sl);
        prevBlock->size += (blk->size & BLOCK_SIZE) + BLOCK_OVERHEAD;
        blk = prevBlock;
    }

    tlsfMapping(blk->size, &fl, &sl);
    insertFreeBlock(blk,tlsf,fl,sl);
    blk->size |= BLOCK_FREE;

    nextBlock = getNextBlock(blk);
    nextBlock->prev = blk;
}

void tlsfPrintBlock(struct TLSFBlock *blk)
{
    if (blk->size & BLOCK_FREE)
        printk("Free ");
    else 
        printk("Used ");
    printk("Addr:%x, Size:%d, Prev:%x\n",blk,blk->size & BLOCK_SIZE,blk->prev);
}

void tlsfDump(struct TLSFHeader *tlsf)
{
    struct TLSFBlock *blk;

    printk("Heap Addr:%x, Size:%d, Used:%d\n",
            tlsf, tlsf->maxSize, tlsf->usedSize);

    blk = tlsf->blockHead;
    while (blk!=tlsf->blockEnd) {
        tlsfPrintBlock(blk);
        blk = getNextBlock(blk);
    };
}

