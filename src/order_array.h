#ifndef ORDER_ARRAY_H
#define ORDER_ARRAY_H

#include "common.h"

typedef void* type_t;

typedef s8int (*less_than_func_t)(type_t, type_t);

typedef struct {
    type_t *index;
    u32int sz;
    u32int max_size;
    less_than_func_t less_than_func;
} order_array_t;

order_array_t create_order_array(u32int max_size, less_than_func_t func);

order_array_t place_order_array(void *addr, u32int max_size, less_than_func_t func);

void insert_order_array(type_t item, order_array_t *array);

void remove_order_array(u32int idx, order_array_t *array);

type_t item_order_array(u32int idx, order_array_t *array);

void dump_order_array(order_array_t *array);

s8int standard_less_than_func(type_t a, type_t b);

#endif
