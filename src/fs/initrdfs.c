#include "initrdfs.h"
#include "kheap.h"

fs_driver_t *driver;

vnode_t *initrd_node;
vnode_t *kbd_node;
vnode_t *file_nodes;
u32int nfiles;
initrdfs_header_t *initrd_header;
initrdfs_file_header_t *initrd_file_header;

u32int initrd_read(vnode_t *node, u32int offset, u32int sz, u8int *buffer);
vnode_t* initrd_readdir(vnode_t *node, u32int i);

vnode_t* initrd_get_root(fs_t *fs)
{
    return initrd_node;
}

void initrdfs_init(fs_driver_t *drv)
{
}

void initrdfs_cleanup(fs_driver_t *drv)
{
}

fs_t* initrdfs_createfs(fs_driver_t *drv, vnode_t *dev, u32int flags, void *data)
{
    // create root vnode;
    initrd_node = (vnode_t *)kmalloc(sizeof(vnode_t));
    memset(initrd_node, 0, sizeof(vnode_t));
    strcpy(initrd_node->name,"initrd");
    initrd_node->flags = VFS_DIRECTORY;
    initrd_node->read = initrd_read;
    initrd_node->readdir = initrd_readdir;

    // create "/dev" vnode
    kbd_node = (vnode_t *)kmalloc(sizeof(vnode_t));
    memset(kbd_node, 0, sizeof(vnode_t));
    strcpy(kbd_node->name,"kbd");
    kbd_node->flags = VFS_DEV_FILE;
    kbd_node->id = 0x00010000;

    // parse memory begin at (u32int)data, construct other vnodes
    u32int loc = (u32int)data;
    initrd_header = (initrdfs_header_t *)loc;
    nfiles = initrd_header->nfiles;

    if (nfiles) {
        file_nodes = (vnode_t *)kmalloc(sizeof(vnode_t) * nfiles);
        memset(file_nodes, 0, sizeof(vnode_t) * nfiles);
        initrd_file_header = (initrdfs_file_header_t *)(loc + sizeof(initrdfs_header_t));
    } else {
        file_nodes = 0;
        initrd_file_header = 0;
    }

    u32int i;

    for (i=0; i<nfiles; i++) {
        ASSERT(initrd_file_header[i].magic == INITRDFS_MAGIC);
        strcpy(file_nodes[i].name,initrd_file_header[i].name);

        file_nodes[i].length = initrd_file_header[i].length;
        file_nodes[i].flags = VFS_FILE;
        file_nodes[i].id = i;
        file_nodes[i].read = initrd_read;
        file_nodes[i].readdir = initrd_readdir;
//        file_nodes[i].finddir = initrd_finddir;

        initrd_file_header[i].offset += loc;
    }


    // return file system instance
    fs_t *fs = (fs_t*)kmalloc(sizeof(fs_t));
    memset(fs, 0, sizeof(fs_t));
    fs->get_root = &initrd_get_root;

    return fs;
}

void initrdfs_removefs(fs_driver_t *drv, fs_t *fs)
{
}

void module_initrdfs_init()
{
    driver = (fs_driver_t *)kmalloc(sizeof(fs_driver_t));
    memset(driver, 0, sizeof(fs_driver_t));

    strcpy(driver->name, "initrdfs");
    driver->init = &initrdfs_init;
    driver->cleanup = &initrdfs_cleanup;
    driver->createfs = &initrdfs_createfs;

    register_fs_driver(driver);
}

void module_initrdfs_cleanup()
{
}

u32int initrd_read(vnode_t *node, u32int offset, u32int sz, u8int *buffer)
{
    initrdfs_file_header_t file_header = initrd_file_header[node->id];
    if (offset < file_header.length) {
        if (offset + sz > file_header.length) 
            sz = file_header.length - offset;
        memcpy(buffer, (u8int*)(file_header.offset + offset), sz);
        return sz;
    }
    return 0;
}

vnode_t* initrd_readdir(vnode_t *node, u32int i)
{
    if (node == initrd_node) {
        if (i==0) {
            return kbd_node;
        } else if (i-1 < nfiles) {
            return &file_nodes[i-1];
        }
    }

    return 0;
}
