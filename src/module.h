#ifndef MODULE_H
#define MODULE_H 

#include "common.h"
#define MEXPORT(sym) \
    static u32int __msym_##sym   \
                __attribute__((unused))    \
                __attribute__((section("__msymtab")))   \
                = (u32int)&sym;
#define MODULE_INIT(sym) MEXPORT(sym)
#define MODULE_CLEANUP(sym) MEXPORT(sym)

#define MAX_SYMBOLS 1000

typedef struct {
    char *name;
    u32int value;
} symbol_t;

typedef struct {
    u32int len;
    symbol_t symbol[MAX_SYMBOLS];
} symtab_t;

typedef void (*module_init_func)();
typedef void (*module_cleanup_func)();

typedef struct module_struct {
    char name[MAX_NAME_LEN];
    char path[MAX_NAME_LEN];
    u32int addr;
    u32int size;
    symtab_t *symtab;

    module_init_func init;
    module_cleanup_func cleanup;

    struct module_struct *next;
} module_t;

extern symtab_t *ksyms;

module_t* load_module(char *path);
void unload_module(module_t *m);

void init_module_system(u32int num, u32int size, u32int addr, u32int strndx);

#endif /* MODULE_H */
