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
#include "paging.h"

u64int totalMemory(); /* in KB */

u64int availMemory(); /* in KB */

void initMemoryManagement(u64int upperMemKB, u64int freePMemStartAddr);

/*-----------------------------------------------------------------------------
 *  Allocate physically continous memory.
 *  Return the virtual address of allocated region.
 *  If physicalAddr != 0, the physical address is putted into.
 *-----------------------------------------------------------------------------*/
u64int kMallocEx(u64int size, u64int pageAligned, u64int *physicalAddr);

u64int kMalloc(u64int size);

void kFree(void *addr);

#endif /* KMM_H */
