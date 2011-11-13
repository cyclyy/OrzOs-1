/*
 * =============================================================================
 *
 *       Filename:  bootinfo.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2011年05月18日 10时47分43秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#include "bootinfo.h"

struct BootInfo *globalBI = 0;

struct BootInfo *getBootInfo()
{
    return globalBI;
}

void setBootInfo(struct BootInfo *bi)
{
    globalBI = bi;
}
