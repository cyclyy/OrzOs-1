/*
 * =============================================================================
 *
 *       Filename:  mm.h
 *
 *    Description:  Memory Management System
 *
 *        Version:  1.0
 *        Created:  2011年05月14日 11时24分19秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#ifndef KMM_H
#define KMM_H

#include "sysdef.h"

#define ROUND_PAGE_ALIGN(x) ((x+PAGE_SIZE-1) & (~0 - PAGE_SIZE + 1))
#define FLOOR_PAGE_ALIGN(x) (x & (~0 & PAGE_SIZE))
#define ROUND_MEM_ALIGN(x) ((x+PTR_SIZE-1) & (~0 - PTR_SIZE + 1))

#define VADDR_TO_PADDR(x) ((((u64int)x)>=CODE_LOAD_ADDR)?(((u64int)x)-CODE_LOAD_ADDR):(((u64int)x)-HEAP_START_ADDR))
#define PADDR_TO_VADDR(x) (((u64int)x)+HEAP_START_ADDR)

struct PageTable {
    u64int page[512];
};

struct PageDirectory {
    u64int pt[512];
};

struct PageDirectoryPointer {
    u64int pd[512];
};

struct PML4E {
    u64int pdp[512];
};

u64int totalMemory(); /* in KB */

u64int availMemory(); /* in KB */

struct PML4E *getPML4E();

void initMemoryManagement(u64int upperMemKB, u64int freePMemStartAddr);

/*-----------------------------------------------------------------------------
 *  Map n pages start from vaddr to paddr
 *-----------------------------------------------------------------------------*/
void mapPagesVtoP(u64int vaddr, u64int paddr, u64int n, struct PML4E *pml4e);

/*-----------------------------------------------------------------------------
 *  Unmap n pages start from vaddr
 *-----------------------------------------------------------------------------*/
void unmapPages(u64int vaddr, u64int n);

/*-----------------------------------------------------------------------------
 *  Allocate physically continous memory.
 *  Return the virtual address of allocated region.
 *  If physicalAddr != 0, the physical address is putted into.
 *-----------------------------------------------------------------------------*/
u64int kMallocEx(u64int size, u64int pageAligned, u64int *physicalAddr);

u64int kMalloc(u64int size);

void kFree(void *addr);

#endif /* KMM_H */
