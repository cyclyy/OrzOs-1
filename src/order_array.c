#include "order_array.h"
#include "kheap.h"
#include "screen.h"

order_array_t create_order_array(u32int max_size, less_than_func_t func)
{
    order_array_t tmp;
    tmp.sz = 0;
    tmp.max_size = max_size;
    tmp.less_than_func = func;
    tmp.index = (type_t *)kmalloc(max_size * sizeof(type_t));
    memset(tmp.index, 0, max_size * sizeof(type_t));
    return tmp;
}

order_array_t place_order_array(void *addr, u32int max_size, less_than_func_t func)
{
    order_array_t tmp;
    tmp.sz = 0;
    tmp.max_size = max_size;
    tmp.less_than_func = func;
    tmp.index = (type_t *)addr;
    memset(tmp.index, 0, max_size * sizeof(type_t));
    return tmp;
}

void insert_order_array(type_t item, order_array_t *array)
{
    ASSERT(array);
    u32int i = 0;

    if (array->sz == array->max_size)
        PANIC("insert failed, array is full");

    while ((i < array->sz) && array->less_than_func(array->index[i], item) )
        i++;
    if (i == array->sz) {
        array->index[i] = item;
        array->sz++;
    } else {
        u32int j;
        for (j=array->sz; j>i; j--) {
            array->index[j] = array->index[j-1];
        }
        array->index[i] = item;
        array->sz++;
    }
}

void remove_order_array(u32int idx, order_array_t *array)
{
    ASSERT(array);
    ASSERT(idx < array->sz);
    u32int j;
    for (j=idx; j<(array->sz - 1); j++)
        array->index[j] = array->index[j+1];
    array->sz--;
}

type_t item_order_array(u32int idx, order_array_t *array)
{
    ASSERT(array);
    ASSERT(idx < array->sz);
    return array->index[idx];
}

void dump_order_array(order_array_t *array)
{
    scr_putp("=== dump order_array", array);
    scr_puts("size:");
    scr_putn(array->sz);
    scr_puts(", max_size:");
    scr_putn(array->max_size);
    scr_puts("\n");
    u32int i;
    for (i=0; i<array->sz; i++) {
        scr_puthex(array->index[i]);
        scr_puts(" ");
    }
    if (array->sz)
        scr_puts("\n");
    scr_puts("=== end dump order_array\n");
}

s8int standard_less_than_func(type_t a, type_t b)
{
    return (a<b)?1:0;
}
