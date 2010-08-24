#include "vfs.h"
#include "kheap.h"
#include "screen.h"
#include "cdev.h"

vnode_t *vfs_root = 0;
fs_driver_t *fs_drivers = 0;

void init_vfs()
{
    vfs_root = (vnode_t *)kmalloc(sizeof(vnode_t));
    memset(vfs_root, 0, sizeof(vnode_t));
    vfs_root->flags = VFS_DIRECTORY;
    strcpy(vfs_root->name, "root");
}

void register_fs_driver(fs_driver_t *driver)
{
    if (!driver) 
        return;

    if (fs_drivers) {
        driver->next = fs_drivers;
    }
    fs_drivers = driver; 
}

void unregister_fs_driver(fs_driver_t *driver)
{
    if (!driver) 
        return;

    if (driver == fs_drivers) {
        fs_drivers = 0;
    } else {
        fs_driver_t *p = fs_drivers;

        while (p) {
            if (p->next == driver) {
                p->next = driver->next;
            }
            p = p->next;
        }
    }
}

fs_driver_t* get_fs_driver_byname(char *name)
{
    fs_driver_t *driver = fs_drivers;
    while (driver && (strcmp(driver->name,name) != 0) )
        driver = driver->next;
    return driver;
}

u32int vfs_read(vnode_t *node, u32int offset, u32int sz, u8int *buffer)
{
    /*
     *printk("vfs_read offset: %p, size: %d, buffer: %p\n",
     *        offset, sz, buffer);
     */
    if (!node)
        return 0;
    if (node->read)
        return node->read(node,offset,sz,buffer);

    return 0;
}

u32int vfs_write(vnode_t *node, u32int offset, u32int sz, u8int *buffer)
{
    if (!node)
        return 0;

    if (node->write)
        return node->write(node,offset,sz,buffer);

    return 0;
}

void vfs_open(vnode_t *node)
{
    if (!node)
        return;
    if (node->flags & VFS_DEV_FILE) {
        cdev_t *cdev = find_cdev(node->id);
        if (cdev) {
            node->open = cdev->open;
            node->read = cdev->read;
            node->write = cdev->write;
            node->close = cdev->close;
        }
    }
    if (node->open)
        node->open(node);
}

void vfs_close(vnode_t *node)
{
    if (!node)
        return;
    if (node->close)
        node->close(node);
}

vnode_t* vfs_readdir(struct vnode *vnode, u32int i)
{
    if (!vnode)
        return 0;

    if (vnode->flags & VFS_MOUNTPOINT) {
        vnode = vnode->ptr;

        if (!vnode) {
            return 0;
        }
    }

    if ( (vnode->flags & VFS_DIRECTORY) && vnode->readdir) {
        return vnode->readdir(vnode, i);
    }
    return 0;
}

vnode_t* vfs_finddir(struct vnode *vnode, char *name)
{
    if (!vnode)
        return 0;

    if (vnode->flags & VFS_MOUNTPOINT) {
        vnode = vnode->ptr;

        if (!vnode)
            return 0;
    }

    // if vnode's file system driver only implement readdir, then enum the child vnodes
    if (vnode->flags & VFS_DIRECTORY) {
        if (vnode->finddir) 
            return vnode->finddir(vnode, name);
        else if (vnode->readdir) {
            u32int i = 0;
            vnode_t *tmp = 0;
            while ( (tmp = vnode->readdir(vnode, i++)) ) {
                if (strcmp(tmp->name, name) == 0)
                    return tmp;
            }
        }
    }

    return 0;
}

vnode_t* vfs_lookup(vnode_t *node, char *path)
{
    /*
    if ((node->flags & FS_DIRECTORY) && node->lookup) {
        return node->lookup(node,name);
    }
    */
    if (!node || !path || (path[0] == 0) || (path[0] != '/') ) 
        return 0;

    vnode_t *tmp = node;
    char s[MAX_NAME_LEN];
    u32int len = strlen(path);

    if (len == 1)
        return node;

    u32int i = 1;
    u32int j = 0;

    do {
        s[j++] = path[i++];

        if ((path[i] == '/') || (path[i] == 0)) {
            s[j] = 0;
            /*printk("vfs_lookup: %s\n",s);*/
            if ((tmp->flags & VFS_MOUNTPOINT)) {
                tmp = tmp->ptr;
            }
            if (tmp->flags & VFS_DIRECTORY) {
                tmp = vfs_finddir(tmp, s);
            } else {
                tmp = 0;
            }
            j = 0;
            i++;
        }
    } while (tmp && (i<len));

    return tmp;
}

void vfs_mount(vnode_t *node, fs_t *fs)
{
    if (!node || !fs)
        return;
    vnode_t *to_mount = fs->get_root(fs);

    if ( (node->flags & VFS_DIRECTORY) && !(node->flags & VFS_MOUNTPOINT) ) {
        node->flags |= VFS_MOUNTPOINT;
        to_mount->covered = node;
        node->ptr = to_mount;
    } else {
        scr_puts("vfs_mount failed");
    }
}

void syscall_mount(char *src, char* dst, u32int flags, void *data)
{
}

void dump_vnode(vnode_t *node)
{
    if (!node) {
        scr_puts("vnode null\n");
        return;
    }
    scr_puts("vnode \"");
    scr_puts(node->name);
    scr_puts("\" :");
    
    if (node->flags & VFS_FILE)
        scr_puts(" VFS_FILE ");
    if (node->flags & VFS_DIRECTORY)
        scr_puts(" VFS_DIRECTORY ");
    if (node->flags & VFS_MOUNTPOINT)
        scr_puts(" VFS_MOUNTPOINT ");

    scr_puts("\n");
}
