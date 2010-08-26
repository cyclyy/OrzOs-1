#include "initrdfs.h"
#include "kheap.h"
#include "screen.h"
#include "file.h"
#include "vfs.h"

fs_driver_t *driver;

vnode_t *initrd_node;
vnode_t *kbd_node;
vnode_t *file_nodes;
u32int nfiles;
initrdfs_header_t *initrd_header;
initrdfs_file_header_t *initrd_file_header;

s32int initrd_read(file_t *f, u32int offset, u32int sz, u8int *buffer);
vnode_t** initrd_subnodes(vnode_t *node);

struct file_operations initrd_fops = {
    .read = &initrd_read
};

struct vnode_operations initrd_vops = {
    .subnodes = &initrd_subnodes
};

vnode_t* initrdfs_get_root(fs_t *fs)
{
    return initrd_node;
}

vnode_t* initrdfs_lookup(fs_t *fs, char *path)
{
    if (strcmp(path,"/") == 0) {
        return initrd_node;
    } else if (strcmp(path+1, kbd_node->name) == 0) {
        return kbd_node;
    } else {
        u32int i;
        for (i=0; i<nfiles; i++) {
            if (strcmp(path+1, file_nodes[i].name)==0) {
                return &file_nodes[i];
            }
        }
    }
    return 0;
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
    initrd_node->v_ops = &initrd_vops;

    // create "/dev" vnode
    kbd_node = (vnode_t *)kmalloc(sizeof(vnode_t));
    memset(kbd_node, 0, sizeof(vnode_t));
    strcpy(kbd_node->name,"hda");
    kbd_node->flags = VFS_DEV_FILE;
    kbd_node->dev_id = 0x400000;

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
        file_nodes[i].ino = i;
        file_nodes[i].f_ops = &initrd_fops;
//        file_nodes[i].finddir = initrd_finddir;

        initrd_file_header[i].offset += loc;
    }


    // return file system instance
    fs_t *fs = (fs_t*)kmalloc(sizeof(fs_t));
    memset(fs, 0, sizeof(fs_t));
    fs->get_root = &initrdfs_get_root;
    fs->lookup = &initrdfs_lookup;

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

s32int initrd_read(file_t *f, u32int offset, u32int sz, u8int *buffer)
{
    initrdfs_file_header_t file_header = initrd_file_header[f->vnode->ino];
    if (offset < file_header.length) {
        if (offset + sz > file_header.length) 
            sz = file_header.length - offset;
        memcpy(buffer, (u8int*)(file_header.offset + offset), sz);
        return sz;
    }
    return 0;
}

vnode_t** initrd_subnodes(vnode_t *node)
{

    if (node == initrd_node) {
        vnode_t **ret = (vnode_t**)kmalloc((nfiles+2)*sizeof(vnode_t*));
        u32int i;
        ret[0] = kbd_node;
        for (i=0; i<nfiles; i++) {
            ret[i+1] = &file_nodes[i];
        }
        ret[nfiles+1] = 0;
        return ret;
    }

    return 0;
}
