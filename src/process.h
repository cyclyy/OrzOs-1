/*
 * =============================================================================
 *
 *       Filename:  process.h
 *
 *    Description:  Process Management
 *
 *        Version:  1.0
 *        Created:  2011年05月18日 09时38分35秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "sysdef.h"
#include "vmm.h"

#define PROC_STATE_READY    1
#define PROC_STATE_WAIT     2

struct Process {
    u64int pid;
    u64int priority;
    u64int state;
    u64int ticks;
    u64int slices;
    struct VM *vm;
    u64int rip, rsp, rbp;
    struct Process *next, *prev;
    struct Process *rqNext, *rqPrev;
};

void initMultitasking();

void schedule();

#endif
