#include "vm.h"
#include "paging.h"
#include "kheap.h"
#include "screen.h"
#include "file.h"
#include "task.h"

vma_t* get_vma_cover(vm_t *vm, u32int start)
{
    vma_t *vma;
    for (vma = vm->vmas; vma; vma = vma->next) {
        if ((vma->start <= start) && (start < vma->end)) 
            break;
    }

    return vma;
}

vma_t* get_vma_cover2(vm_t *vm, u32int start, u32int end)
{
    if (start >= end)
        return 0;

    vma_t *vma;
    for (vma = vm->vmas; vma; vma = vma->next) {
        if ((vma->start <= start) && (end <= vma->end)) 
            break;
    }

    return vma;
}

void insert_vma(vm_t *vm, vma_t *vma)
{
    /*printk("insert_vma %p -> %p\n", vma->start, vma->end);*/
    if (!vm || !vma)
        return;

    vma->vm = vm;

    if (vm->vmas == 0) {
        vm->vmas = vma;
        vma->prev = vma->next = 0;
    } else if (vm->vmas->start > vma->start) {
        vma->prev = 0;
        vma->next = vm->vmas;
        vm->vmas->prev = vma;
        vm->vmas = vma;
    } else {
        vma_t *p = vm->vmas;

        while (p->next && (p->next->start < vma->start))
            p = p->next;

        vma->prev = p;
        vma->next = p->next;

        if (p->next)
            p->next->prev = vma;
        p->next = vma;
    }
}

void remove_vma(vm_t *vm, vma_t *vma)
{
    if (!vm || !vma)
        return;
    if (vma->prev) 
        vma->prev->next = vma->next;

    if (vma->next) 
        vma->next->prev = vma->prev;

    vma->next = vma->prev = 0;

}

void free_vmas(vm_t *vm)
{
    if (!vm->vmas)
        return;

    vma_t *vma;
    while ((vma = vm->vmas->next)) {
        remove_vma(vm, vm->vmas->next);
        kfree(vma);
    }

    kfree(vm->vmas);

    vm->vmas = 0;
}

u32int get_vm_free_start(vm_t *vm, u32int start, u32int size)
{
    if (!vm || !size)
        return 0;

    if (!vm->vmas)
        return start;

    vma_t *p = vm->vmas;

    do {
        if (start+size < p->start) {
            return start;
        }
        start = p->end;
        p = p->next;
    } while (p);

    return 0;
}

void vma_nopage(vma_t *vma, u32int vaddr)
{
    if (!vma)
        return;

    /*printk("cr2 %p\n", current_task->dir->addr);*/
    /*MAGIC_BREAK();*/
    alloc_frame(get_page(vaddr, 1, current_task->dir), 1, vma->flags & VMA_WRITE);
    switch_page_directory(current_task->dir);

    if (vma->flags & VMA_ANON) {
        if (vma->flags & VMA_STACK) {
            vma->vm->end_stack -= 0x1000;
            vma->start -= 0x1000;
        }
        return;
    }

    if (vma->flags & VMA_FILE) {
        file_mapping_t *f_map = (file_mapping_t*)vma->priv;
        u32int size = 0x1000;

        u32int off = vaddr - vma->start;
        if (off >= f_map->size) {
            memset((void*)vaddr, 0, size);
        } else {
            file_lseek(f_map->file, f_map->offset + off, SEEK_SET);
            if (off + size > f_map->size) {
                /*printk("file_read: %p, size: %d\n", vaddr, f_map->size - off);*/
                /*printk("memset: %p, size: %d\n", vma->start+f_map->size, size + off - f_map->size);*/
                file_read(f_map->file, (u8int*)vaddr, f_map->size - off);
                memset((void*)(vma->start + f_map->size), 0, size + off - f_map->size);
            } else {
                /*printk("file_read: %p, size: %d\n", vaddr, size);*/
                file_read(f_map->file, (u8int*)vaddr, size);
            }
        }
    }

}

void vma_freepages(vma_t *vma)
{
    if (!vma)
        return;

    u32int i;
    for (i=vma->start; i<vma->end; i+=PAGE_SIZE) {
        /*printk("free_page %p\n", i);*/
        INVLPG(i);
        free_page( get_page(i,0,current_task->dir) );
    }
}

void vma_write(vma_t *vma, u32int vaddr, u32int size)
{
    if (!vma || vma->flags & VMA_ANON)
        return;

    if (vma->flags & VMA_FILE) {
        file_mapping_t *f_map = (file_mapping_t*)vma->priv;

        if (!f_map)
            return;
    }
}

vma_t *clone_vma(vma_t *vma)
{
    if (!vma)
        return 0;

    vma_t *ret;

    ret = (vma_t*)kmalloc(sizeof(vma_t));
    memcpy(ret, vma, sizeof(vma_t));

    if (vma->flags & VMA_FILE) {
        ret->priv = clone_file_mapping((file_mapping_t*)vma->priv);
    }

    return ret;
}

vm_t *clone_vm(vm_t *vm)
{
    if (!vm)
        return 0;

    vm_t *ret;

    ret = (vm_t*)kmalloc(sizeof(vm_t));
    memcpy(ret, vm, sizeof(vm_t));
    ret->vmas = 0;

    vma_t *vma = vm->vmas;

    while(vma) {
        vma_t *new_vma = clone_vma(vma);
        insert_vma(ret, new_vma);
        vma = vma->next;
    }

    return ret;
}


