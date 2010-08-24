#include "kheap.h"
#include "common.h"
#include "screen.h"
#include "paging.h"

extern u32int end;

u32int placement_addr = (u32int)&end;

extern page_directory_t *kernel_dir;

heap_t *kheap = 0;

void expand(u32int new_size, heap_t *heap)
{
    ASSERT(heap);

    u32int old_size = heap->end_addr - heap->start_addr;

    if (new_size <= old_size)
        return;

    // page align
    if (new_size & 0xfff)
        new_size = (new_size & 0xfffff000) + 0x1000;

    if (new_size >= heap->max_end_addr - heap->start_addr)
        PANIC("heap full");

    u32int i = old_size;

//    scr_putp("alloc frame of page from",heap->start_addr+old_size);
//    scr_putp("alloc frame of page to",heap->start_addr+new_size);

    while (i < new_size) {
        alloc_frame(get_page(heap->start_addr+i, 1, kernel_dir), heap->su, heap->rw);
        i += 0x1000;
    }

    heap->end_addr = heap->start_addr + new_size;

}

void compact(u32int new_size, heap_t *heap)
{
    ASSERT(heap);

    u32int old_size = heap->end_addr - heap->start_addr;

    // page align
    if (new_size & 0xfff)
        new_size = (new_size & 0xfffff000) + 0x1000;

    if (new_size >= old_size)
        return;

    u32int i = new_size;

    //scr_putp("free frame of page from",heap->start_addr+new_size);
    //scr_putp("free frame of page to",heap->start_addr+old_size);
    while (i < old_size) {
        page_t *page = get_page(heap->start_addr+i, 0, kernel_dir);
        ASSERT(page);
        free_page(page);
        i += 0x1000;
    }

    heap->end_addr = heap->start_addr + new_size;
}

void free(void *p, heap_t *heap)
{
    ASSERT(heap);

    if (p == 0)
        return;

    ASSERT((u32int)p >= heap->start_addr + sizeof(header_t));

    header_t *header = (header_t *)(p - sizeof(header_t));
    footer_t *footer = (footer_t *)(p + header->sz);


    ASSERT(header->magic == HEAP_MAGIC);
    ASSERT(footer->magic == HEAP_MAGIC);
    ASSERT(footer->header == header);

    u32int i = 0;
    u8int found = 0;
    
    for (i = 0; i<heap->alloc_addr.sz; i++) {
        //scr_putp("alloc_addr.index[i]",(void *)heap->alloc_addr.index[i]);
        if (heap->alloc_addr.index[i] == (type_t)header) {
            remove_order_array(i,&heap->alloc_addr);
            found = 1;
            break;
        }
    }

    ASSERT(found);

    // doesn't compact kheap
    /*
    u32int used_upper_addr = heap->start_addr;

    if (heap->alloc_addr.sz) {
        used_upper_addr = (u32int)heap->alloc_addr.index[ heap->alloc_addr.sz - 1 ];
        header_t *header = (header_t *)used_upper_addr;
        used_upper_addr += sizeof(header_t) + header->sz;
        used_upper_addr += sizeof(footer_t);

        if (used_upper_addr & 0xfff) {
            used_upper_addr &= 0xfffff000;
            used_upper_addr += 0x1000;
        }


        // scr_putp("used_upper_addr", (void *)used_upper_addr);
        // scr_putp("heap->start_addr", (void *)heap->start_addr);
        // scr_putp("heap->end_addr", (void *)heap->end_addr);
        // scr_putp("KHEAP_INIT_END", (void *)KHEAP_INIT_END);

        if (used_upper_addr < KHEAP_INIT_END)
            used_upper_addr = KHEAP_INIT_END;

        if (used_upper_addr < heap->end_addr) {
             *u32int new_size = used_upper_addr - heap->start_addr;
             *scr_putp("compact heap to", (void *)new_size);
             *compact(new_size, kheap);
        }

    }
    */

}

// return the start address of first fitted hole, sz include sizeof(header_t)+sizeof(footer_t), return -1 if can't find
u32int first_fit(u32int sz, u8int align, heap_t *heap)
{
    ASSERT(heap);

    u32int nalloc = heap->alloc_addr.sz;
    u32int free_start_addr, free_end_addr;

    u32int offset = 0;

    //dump_order_array(&heap->alloc_addr);

    if (nalloc) {
        free_start_addr = heap->start_addr;
//        scr_putp("heap->start_addr",heap->start_addr);
        u32int i = 0;
        for (i=0; i<nalloc; i++) {
//              scr_putp("alloc_addr.index[i]",(void *)heap->alloc_addr.index[i]);
//            scr_putp("free_start_addr", (void *)free_start_addr);
            if (free_start_addr < (u32int)heap->alloc_addr.index[i]) {
//                scr_putp("free_start_addr", (void *)free_start_addr);
                free_end_addr = (u32int)heap->alloc_addr.index[i];
//                scr_putp("free_end_addr", (void *)free_end_addr);

                if (align && ((free_start_addr + sizeof(header_t)) & 0xfff) ) {
                    offset = 0x1000 - ((free_start_addr + sizeof(header_t)) & 0xfff);
                    free_start_addr += offset;
                    
                    if (free_start_addr >= free_end_addr)
                        free_start_addr = free_end_addr;
                }
                if (free_end_addr - free_start_addr >= sz) {
                    return free_start_addr;
                }
            }

            header_t *header = (header_t *)heap->alloc_addr.index[i];

            free_start_addr = (u32int)heap->alloc_addr.index[i] + sizeof(header_t) + header->sz + sizeof(footer_t);

            /*
            scr_putp("free_start_addr", (void *)free_start_addr);
            scr_putp("sizeof footer", (void *)sizeof(footer_t));
            */

        }

        header_t *header = (header_t *)heap->alloc_addr.index[nalloc-1];

        free_start_addr = (u32int)heap->alloc_addr.index[nalloc-1] + sizeof(header_t) + header->sz + sizeof(footer_t);

        free_end_addr = heap->end_addr;
//        scr_putp("last free_start_addr", (void *)free_start_addr);
//        scr_putp("last free_end_addr", (void *)free_end_addr);

        if (align && ((free_start_addr + sizeof(header_t)) & 0xfff) ) {
            offset = 0x1000 - ((free_start_addr + sizeof(header_t)) & 0xfff);
            free_start_addr += offset;
            
            if (free_start_addr >= free_end_addr)
                free_start_addr = free_end_addr;
        }

        if (free_end_addr - free_start_addr >= sz) {
            return free_start_addr;
        }
    } else {
        free_start_addr = heap->start_addr;
        free_end_addr = heap->end_addr;
        if (align && ((free_start_addr + sizeof(header_t)) & 0xfff) ) {
            offset = 0x1000 - ((free_start_addr + sizeof(header_t)) & 0xfff);
            free_start_addr += offset;
            
            if (free_start_addr >= free_end_addr)
                return -1;

        }
        if (free_end_addr - free_start_addr >= sz) {
            return free_start_addr;
        }
    }

    return (u32int)-1;
}

u32int kmalloc_int(u32int sz, u8int align, u32int *phys)
{
    if (!sz)
        return 0;

    u32int real_size = sizeof(header_t) + sz + sizeof(footer_t);

    if (kheap) {
        u32int hole_addr = first_fit(real_size, align, kheap);
//        scr_putp("alloc size ",(void *)sz);
//        scr_putp("found hole_addr: ",(void *)hole_addr);
        if (hole_addr != (u32int)-1) {
            header_t *header = (header_t *)hole_addr;
            header->magic = HEAP_MAGIC;
            header->sz = sz;
            footer_t *footer = (footer_t*)(hole_addr + sizeof(header_t) + sz);
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_order_array((type_t)hole_addr, &kheap->alloc_addr);

            u32int ret = hole_addr + sizeof(header_t);

            if (phys) {
                page_t *page = get_page(ret,0,kernel_dir);

                ASSERT(page);
                
                *phys = (page->addr*0x1000) + (ret & 0xfff);
            }

//            scr_putp("kmalloc return", ret);
            return ret;
        } else {
            // expand the heap
            u32int new_size = kheap->end_addr - kheap->start_addr + real_size;
            printk("expanding kheap to %p\n", kheap->end_addr + real_size);

            expand(new_size, kheap);

            return kmalloc_int(sz,align,phys);
        }
    } else {
        if (align && (placement_addr & 0xfff)) {
            placement_addr = (placement_addr & 0xfffff000) + 0x1000;
        }

        if (phys) {
            *phys = placement_addr;
        }

        u32int ret = placement_addr;

        placement_addr += sz;

        return ret;
    }

    return 0;
}

u32int kmalloc_a(u32int sz)
{
    return kmalloc_int(sz, 1, 0);
}

u32int kmalloc_p(u32int sz, u32int *phys)
{
    return kmalloc_int(sz, 0, phys);
}

u32int kmalloc_ap(u32int sz, u32int *phys)
{
    return kmalloc_int(sz, 1, phys);
}

u32int kmalloc(u32int sz)
{
    return kmalloc_int(sz, 0, 0);
}

void kfree(void *p)
{
    if (kheap)
        free(p, kheap);
}

heap_t *create_heap(u32int start_addr, u32int end_addr, u32int max_end_addr, u32int supervisor, u32int writable)
{
    heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));

    u32int start =start_addr;

    start += sizeof(type_t) * KHEAP_MAX_IDX;

    if (start & 0xfff) {
        start &= 0xfffff000;
        start += 0x1000;
    }

    heap->alloc_addr = place_order_array((type_t)start_addr, KHEAP_MAX_IDX, &standard_less_than_func);

    heap->start_addr = start;
    heap->end_addr = end_addr;
    heap->max_end_addr = max_end_addr;
    heap->su = supervisor;
    heap->rw = writable;
    
    return heap;
}

