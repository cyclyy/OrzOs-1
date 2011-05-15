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

#include "sysdef.h"

#define HIGHMEM_START_ADDR 0x100000
#define HEAP_START_ADDR 0xffffff0000000000      /* start from last 1024GB */
#define CODE_LOAD_ADDR 0xfffffffC0000000        /* kernel loaded here */

#define PAGE_SIZE 4096
#define PTR_SIZE 8
#define ROUND_PAGE_ALIGN(x) ((x+PAGE_SIZE-1) & (~0 - PAGE_SIZE + 1))
#define FLOOR_PAGE_ALIGN(x) (x & (~0 & PAGE_SIZE))
#define ROUND_MEM_ALIGN(x) ((x+PTR_SIZE-1) & (~0 - PTR_SIZE + 1))

#define VADDR_TO_PADDR(x) ((x>=CODE_LOAD_ADDR)?(x-CODE_LOAD_ADDR):(x-HEAP_START_ADDR))
#define PADDR_TO_VADDR(x) (x+HEAP_START_ADDR)

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

void initMemoryManagement(u64int upperMemKB, u64int freePMemStartAddr);

void setAddressSpace();

/*-----------------------------------------------------------------------------
 *  Allocate physically continous memory.
 *  Return the virtual address of allocated region.
 *  If physicalAddr != 0, the physical address is putted into.
 *-----------------------------------------------------------------------------*/
u64int kmallocEx(u64int size, u64int pageAligned, u64int *physicalAddr);

void free(u32int addr);

