#include "paging.h"
#include "kmm.h"
#include "util.h"

extern struct TLSFHeader *heap;

#define INVLPG(x) {asm volatile("invlpg %0"::"m"(*(char*)x)); }
#define MAP_ADDR_P_TO_V(x) ((x) ? (heap ? ((u64int)(x) + HEAP_START_ADDR):((u64int)(x) + CODE_LOAD_ADDR)) : 0)
#define MAP_ADDR_V_TO_P(x) ((x) ? (heap ? ((u64int)(x) - HEAP_START_ADDR):((u64int)(x) + CODE_LOAD_ADDR)) : 0)

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
void mapPagesVtoP(u64int vaddr, u64int paddr, u64int n, struct PML4E *pml4e, u64int user)
{
    u64int i, physAddr, doInvlPG = 0;
    u64int *page;
    struct PageTable *pt;
    struct PageDirectory *pd;
    struct PageDirectoryPointer *pdp;

    if (pml4e == getPML4E())
        doInvlPG = 1;

    for (i=0; i<n; i++) {
        pdp = getPDP(pml4e, vaddr);
        if (!pdp) {
            pdp = (struct PageDirectoryPointer *)kMallocEx(4096,1,&physAddr);
            memset(pdp, 0, sizeof(struct PageDirectoryPointer));
            pml4e->pdp[((vaddr>>39) & 511)] = physAddr | 7;
        }
        pd = getPD(pdp,  vaddr);
        if (!pd) {
            pd = (struct PageDirectory *)kMallocEx(4096,1,&physAddr);
            memset(pd, 0, sizeof(struct PageDirectory));
            pdp->pd[(vaddr>>30) & 511] = physAddr | 7;
        }
        pt = getPT(pd, vaddr);
        if (!pt) {
            pt = (struct PageTable *)kMallocEx(4096,1, &physAddr);
            memset(pt, 0, sizeof(struct PageTable));
            pd->pt[(vaddr>>21) & 511] = physAddr | 7;
        }
        page = getPage(pt, vaddr);
        *page = paddr | 3;
        if (user)
            *page |= 4;
        if (doInvlPG) {
            INVLPG(vaddr);
        }
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }
}

/*-----------------------------------------------------------------------------
 *  Unmap n pages start from vaddr
 *-----------------------------------------------------------------------------*/
void unmapPages(u64int vaddr, u64int n, struct PML4E *pml4e)
{
    u64int i, doInvlPG = 0;
    u64int *page;
    struct PageTable *pt;
    struct PageDirectory *pd;
    struct PageDirectoryPointer *pdp;

    DBG("%x,%x",vaddr,n);

    if (pml4e == getPML4E())
        doInvlPG = 1;
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
        if (doInvlPG) {
            INVLPG(vaddr);
        }
        vaddr += PAGE_SIZE;
    }

}

u64int getPAddr(u64int vaddr, struct PML4E *pml4e)
{
    u64int *page;
    struct PageTable *pt;
    struct PageDirectory *pd;
    struct PageDirectoryPointer *pdp;

    pdp = getPDP(pml4e, vaddr);
    if (!pdp)
        return 0;
    pd = getPD(pdp, vaddr);
    if (!pd)
        return 0;
    pt = getPT(pd, vaddr);
    if (!pt)
        return 0;
    page = getPage(pt, vaddr);
    if (*page & PAGE_MASK) {
        return (*page & PAGE_MASK) + (vaddr % PAGE_SIZE);
    } else {
        return 0;
    }
}

void parseVAddr(u64int vaddr, u64int *pml4eIdx, u64int *pdpIdx, u64int *pdIdx, u64int *ptIdx)
{
    *pml4eIdx = (vaddr >> 39) & 511;
    *pdpIdx = (vaddr >> 30) & 511;
    *pdIdx = (vaddr >> 21) & 511;
    *ptIdx = (vaddr >> 12) & 511;
}

u64int buildVAddr(u64int pml4eIdx, u64int pdpIdx, u64int pdIdx, u64int ptIdx)
{
    return ((u64int)1<<39) * pml4eIdx + ((u64int)1<<30)*pdpIdx +(1<<21)*pdIdx +(1<<12)*ptIdx;
}

