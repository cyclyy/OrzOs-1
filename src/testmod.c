#include "testmod.h"
#include "screen.h"
#include "vfs.h"
#include "module.h"
#include "pci.h"

char stra[127];
u32int arr[63] = {1};
extern vnode_t *vfs_root;

void module_testmod_init()
{
    printk("register_pci_driver %p\n", &register_pci_driver);
}

void module_testmod_cleanup()
{
    printk("!!! module_test_cleanup\n");
}

MODULE_INIT(module_testmod_init);
MODULE_CLEANUP(module_testmod_cleanup);
