#include "rmi.h"
#include "x86emu.h"
#include "util.h"
#include "kmm.h"
#include "vmm.h"

static struct VMA *vmaHead = 0;

static X86EMU_pioFuncs pioFuncs = {
    .inb = &inb,
    .inw = &inw,
    .inl = &inl,
    .outb = &outb,
    .outw = &outw,
    .outl = &outl,
};

void initRealModeInterface()
{
    X86EMU_setupPioFuncs(&pioFuncs);
    memset(&M, 0, sizeof M);
    M.mem_base = KERNEL_HEAP_START_ADDR;
    M.mem_size = 0x100000;
    vmaHead = vmaCreate(DYNAMIC_LOWMEM_START, DYNAMIC_LOWMEM_SIZE, VMA_STATUS_FREE);
}

void realModeInterrupt(u8int num)
{
    M.x86.R_SS = 0x6000;
    M.x86.R_ESP = 0x0000;
    M.x86.R_CS = 0x4000;
    M.x86.R_EIP = 0x01;
    *(u8 *)0x40001 = 0xf4; /* HLT, so the emulator knows where to stop */

    X86EMU_prepareForInt(num);
    X86EMU_exec();
}

void *realModeAlloc(u64int size)
{
    struct VMA *v, *vma;
    s64int flags;

    flags = VMA_STATUS_USED | VMA_OWNER_KERNEL | VMA_TYPE_HEAP;
    v = vmaHead;
    while (v) {
        if ((v->flags == VMA_STATUS_FREE) && 
                (v->start + size <= v->start+v->size)) {
            break;
        }
        v = v->next;
    }
    if (!v)
        return 0;
    v->flags = flags;
    if (v->size > size) {
        vma = vmaCreate(v->start+size, v->size - size, VMA_STATUS_FREE);
        vma->next = v->next;
        vma->prev = v;
        if (v->next)
            v->next->prev = vma;
        v->next = vma;
    }
    return (void*)(v->start + KERNEL_HEAP_START_ADDR);
}

void realModeFree(void *ptr)
{
    struct VMA *vma, *v;
    u64int addr;

    addr = (u64int)ptr;
    if ((addr == 0) || (addr < KERNEL_HEAP_START_ADDR + DYNAMIC_LOWMEM_START) || 
            (addr >= KERNEL_HEAP_START_ADDR + DYNAMIC_LOWMEM_END)) {
        return;
    }

    addr -= KERNEL_HEAP_START_ADDR;
    vma = vmaHead;
    while (vma) {
        if ((addr == vma->start) && (vma->flags & VMA_STATUS_FREE))
            break;
            
        vma = vma->next;
    }
    if (!vma) {
        return;
    }
    vma->flags = VMA_STATUS_FREE;
    v = vma->prev;
    if (v && (v->flags & VMA_STATUS_FREE)) {
        if (v->prev) 
            v->prev->next = v->next;
        if (v->next)
            v->next->prev = v->prev;
        vma->start = v->start;
        vma->size += v->size;
        vma->prev = v->prev;
        if (vmaHead == v)
            vmaHead = vma;
        kFree(v);
    }
    v = vma->next;
    if (v && (v->flags & VMA_STATUS_FREE)) {
        if (v->prev) 
            v->prev->next = v->next;
        if (v->next)
            v->next->prev = v->prev;
        vma->size += v->size;
        vma->next = v->next;
        kFree(v);
    }
    return;
}

// vim: sw=4 sts=4 et tw=100
