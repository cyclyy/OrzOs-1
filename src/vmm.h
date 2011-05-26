/*
 * =============================================================================
 *
 *       Filename:  vmm.h
 *
 *    Description:  Manage Address Space
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 22时20分50秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wang Hoi (whoi), fearee@gmail.com
 *        Company:  
 *
 * =============================================================================
 */

#ifndef VMM_H
#define VMM_H

#include "sysdef.h"

#define VMA_STATUS_INVALID  0
#define VMA_STATUS_USED     1
#define VMA_STATUS_FREE     2
#define VMA_STATUS_MASK     3

#define VMA_OWNER_UNKNOWN   0
#define VMA_OWNER_KERNEL    4
#define VMA_OWNER_USER      8
#define VMA_OWNER_MASK      12

#define VMA_TYPE_UNKNOWN    0
#define VMA_TYPE_STACK      16
#define VMA_TYPE_HEAP       32
#define VMA_TYPE_MASK       48

struct VMA {
    u64int start;
    u64int size;
    u64int flags;
    struct VMA *prev;
    struct VMA *next;
};

struct VM {
    u64int cr3;
    struct VMA *vmaHead;
};

struct VM *vmInit();
struct VM *vmCreate();

s64int vmTruncate(struct VM *vm);

s64int vmDestroy(struct VM *vm);

struct VM *vmCopy(struct VM *vm, u64int flags);

s64int vmAddArea(struct VM *vm, u64int start, u64int size, u64int flags);

struct VMA *vmQueryArea(struct VM *vm, u64int addr);

s64int vmRemoveArea(struct VM *vm,struct VMA *vma);

// copy between different address space, dest and src must be page aligned
s64int vmemcpy(struct VM *destVM, void *dest, struct VM *srcVM, void *src, u64int size);

void vmDump(struct VM *vm);

#endif /* VMM_H */

