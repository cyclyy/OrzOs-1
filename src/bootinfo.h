/*
 * =============================================================================
 *
 *       Filename:  bootinfo.h
 *
 *    Description:  Boot Info
 *
 *        Version:  1.0
 *        Created:  2011年05月18日 10时45分53秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#ifndef BOOTINFO_H
#define BOOTINFO_H

#include "sysdef.h"

#define PMEM_CHUNK_FREE     1

struct PhysicalMemoryChunk
{
    u32int recLen;
    u64int start;
    u64int size;
    u32int type;
} __attribute__((packed));

struct BootInfo {
    u64int memory;
    u64int initrdAddr;
    u64int initrdEnd;
    u64int freeMemStartAddr;
    u64int initialStack;
    u64int mmapSize;
    struct PhysicalMemoryChunk *chunks;
};

struct BootInfo *getBootInfo();

void setBootInfo(struct BootInfo *);

#endif
