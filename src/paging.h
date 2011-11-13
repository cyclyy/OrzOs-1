#ifndef PAGING_H
#define PAGING_H

#include "sysdef.h"

#define ROUND_PAGE_ALIGN(x) (((x)+PAGE_SIZE-1) & (~0 - PAGE_SIZE + 1))
#define FLOOR_PAGE_ALIGN(x) ((x) & (~0 & PAGE_SIZE))
#define ROUND_MEM_ALIGN(x) (((x)+PTR_SIZE-1) & (~0 - PTR_SIZE + 1))

/*
#define VADDR_TO_PADDR(x) ((((u64int)x)>=CODE_LOAD_ADDR)?(((u64int)x)-CODE_LOAD_ADDR):(((u64int)x)-KERNEL_HEAP_START_ADDR))
#define PADDR_TO_VADDR(x) (((u64int)x)+KERNEL_HEAP_START_ADDR)
*/
#define PADDR_TO_VADDR(x) ((x) ? ((u64int)(x) + KERNEL_HEAP_START_ADDR) : 0)
#define VADDR_TO_PADDR(x) ((x) ? ((((u64int)(x))>=CODE_LOAD_ADDR)?(((u64int)(x))-CODE_LOAD_ADDR):(((u64int)(x))-KERNEL_HEAP_START_ADDR)) : 0)


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

struct PML4E *getPML4E();
struct PageDirectoryPointer *getPDP(struct PML4E *pml4e, u64int addr);
struct PageDirectoryPointer *getPDP(struct PML4E *pml4e, u64int addr);
struct PageDirectory *getPD(struct PageDirectoryPointer *pdp, u64int addr);
struct PageTable *getPT(struct PageDirectory *pd, u64int addr);
u64int* getPage(struct PageTable *pt, u64int addr);

u64int getPAddr(u64int vaddr, struct PML4E *pml4e);

u64int buildVAddr(u64int pml4eIdx, u64int pdpIdx, u64int pdIdx, u64int ptIdx);

void parseVAddr(u64int vaddr, u64int *pml4eIdx, u64int *pdpIdx, u64int *pdIdx, u64int *ptIdx);

/*-----------------------------------------------------------------------------
 *  Map n pages start from vaddr to paddr
 *-----------------------------------------------------------------------------*/
void mapPagesVtoP(u64int vaddr, u64int paddr, u64int n, struct PML4E *pml4e, u64int user);

/*-----------------------------------------------------------------------------
 *  Unmap n pages start from vaddr
 *-----------------------------------------------------------------------------*/
void unmapPages(u64int vaddr, u64int n, struct PML4E *);



#endif /* PAGING_H */
