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
#define VMA_TYPE_MMAP       64
#define VMA_TYPE_MASK       112

#define VMA_DIR_UP          0
#define VMA_DIR_DOWN        1

struct VMA {
    u64int start;
    u64int size;
    u64int flags;
    struct VMA *prev;
    struct VMA *next;
};

struct VM {
    u64int cr3;
    s64int ref;
    struct VMA *vmaHead;
};

extern struct VM *kernelVM;

struct VM *vmInit();
struct VM *vmCreate();

s64int vmTruncate(struct VM *vm);

s64int vmDestroy(struct VM *vm);

struct VM *vmCopy(struct VM *vm, u64int flags);

s64int vmAddArea(struct VM *vm, u64int start, u64int size, u64int flags);

s64int vmMapArea(struct VM *vm, u64int start, u64int size, u64int flags, u64int paddr);

s64int vmUnmapArea(struct VM *vm, u64int start, u64int size);

u64int vmAllocArea(struct VM *vm, u64int startHint, u64int size, 
        u64int startBound, u64int endBound, u64int direct, u64int flags);

struct VMA *vmQueryAreaByAddr(struct VM *vm, u64int addr);

s64int vmRemoveArea(struct VM *vm,struct VMA *vma);

// copy between different address space, dest and src must be page aligned
u64int vmemcpy(struct VM *destVM, void *dest, struct VM *srcVM, void *src, u64int size);

struct VM *vmRef(struct VM *vm);

s64int vmDeref(struct VM *vm);

void vmDump(struct VM *vm);

struct VMA *vmaCreate(u64int start, u64int size, u64int flags);

extern u64int copyUnsafe(void *dest, void *src, u64int size);

u64int copyFromUser(void *dest, void *src, u64int size);

u64int copyToUser(void *dest, void *src, u64int size);

#endif /* VMM_H */

