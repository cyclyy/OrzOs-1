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

struct BootInfo {
    u64int memory;
    u64int initrdAddr;
    u64int initrdEnd;
    u64int freeMemStartAddr;
    u64int initialStack;
};

struct BootInfo *getBootInfo();

void setBootInfo(struct BootInfo *);

#endif
