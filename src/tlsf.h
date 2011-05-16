/*
 * =============================================================================
 *
 *       Filename:  tlsf.h
 *
 *    Description:  TLSF memory allocator
 *
 *        Version:  1.0
 *        Created:  2011年05月15日 13时15分58秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#ifndef TLSF_H
#define TLSF_H

#include "sysdef.h"

#define MAX_SLI_LOG2 3
#define MAX_SLI (1<<MAX_SLI_LOG2)
#define MAX_FLI 40                              /* manage 1TB memory */
#define FLI_OFFSET 6                           
#define SMALL_BLOCK (1<<(FLI_OFFSET+1))             /* 128 byte */
#define MIN_BLOCK_SIZE (1 << (FLI_OFFSET - MAX_SLI_LOG2)) /* 8 byte */
#define REAL_FLI (MAX_FLI - FLI_OFFSET)     
#define BLOCK_OVERHEAD (sizeof(struct TLSFBlock) - sizeof(struct TLSFFreePtr))
#define SPLIT_THREHOLD (BLOCK_OVERHEAD + MIN(sizeof(struct TLSFFreePtr),MIN_BLOCK_SIZE))

#define BLOCK_SIZE  (~0 - MIN_BLOCK_SIZE + 1)
#define BLOCK_FREE  0x1

#define ROUND_MEM_ALIGN

#define TLSF_SIGNATURE 0xABCDFFFFEEEEDCBA

struct TLSFBlock;

struct TLSFFreePtr {
    struct TLSFBlock *prev;
    struct TLSFBlock *next;
};

struct TLSFBlock {
    u64int signature;
    u64int size;
    struct TLSFBlock *prev;
    union {
        struct TLSFFreePtr free;
        u8int *buffer;
    } ptr;
};

struct TLSFHeader {
    u64int usedSize;
    u64int maxSize;
    struct TLSFBlock *blockHead;
    u64int flBitmap;
    u64int slBitmap[REAL_FLI];                  /* i -> 2^(FLI_OFFSET+i) */
    struct TLSFBlock *matrix[REAL_FLI][MAX_SLI];
};

struct TLSFHeader *tlsfInitHeap(u64int startAddr, u64int size);

struct TLSFBlock *tlsfAlloc(struct TLSFHeader *tlsf, u64int size);

void tlsfFree(struct TLSFHeader *tlsf, struct TLSFBlock *blk);

void tlsfDump(struct TLSFHeader *tlsf);

#endif