#include "vfs.h"
#include "kheap.h"
#include "screen.h"
#include "errno.h"
#include "dev.h"
#include "file.h"

vnode_t     *vfs_root   = 0;
fs_driver_t *fs_drivers = 0;
vfsmount_t  *vfs_mounts = 0;

void init_vfs()
{
    vfs_mounts = (vfsmount_t*)kmalloc(sizeof(vfsmount_t));
    memset(vfs_mounts,0,sizeof(vfsmount_t));
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

/*
u32int vfs_read(vnode_t *node, u32int offset, u32int sz, u8int *buffer)
{
    printk("vfs_read offset: %p, size: %d, buffer: %p\n",
            offset, sz, buffer);
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
*/

vnode_t* vfs_lookup(vnode_t *node, char *path)
{
    if (!vfs_mounts)
        return 0;

    u32int i;
    for (i=vfs_mounts->n-1; i>=0; i--) {
        char *s = strstr(path,vfs_mounts->mounts[i].path);
        if (s==path) {
            fs_t *fs = vfs_mounts->mounts[i].fs;
            if (strcmp(path, vfs_mounts->mounts[i].path)==0)
                return fs->get_root(fs);

            s = path+strlen(vfs_mounts->mounts[i].path);
            if (strcmp(vfs_mounts->mounts[i].path,"/")==0) 
                s--;
            if (fs->lookup)
                return fs->lookup(fs, s);
            else 
                return 0;
        }
    }
    return 0;

    /*
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
    */
}

s32int vfs_mount_root(fs_t *fs)
{
    if (!fs || !vfs_mounts)
        return -1;

    // root must be mounted first
    if (vfs_mounts->n != 0)
        return -1;

    vfs_root = fs->get_root(fs);

    if (vfs_root) {
        vfs_mounts->mounts[0].path = strdup("/");
        vfs_mounts->mounts[0].fs   = fs;
        vfs_mounts->n++;
    } else {
        return -1;
    }

    printk("vfs_mount_root ok\n");
    return 0;
}

s32int vfs_mount(char *path, fs_t *fs)
{
    if (!path || !fs || !vfs_mounts)
        return -1;

    if (vfs_mounts->n >= MAX_VFSMOUNTS)
        return -1;

    if (strcmp(path,"/")==0)
        return vfs_mount_root(fs);

    vnode_t *node     = vfs_lookup(vfs_root, path);
    vnode_t *to_mount = fs->get_root(fs);
    u32int i;

    if (!node || !to_mount)
        return  -EFAULT;

    if ( (node->flags & VFS_DIRECTORY) && !(node->flags & VFS_MOUNTPOINT) ) {
        // add to vfs_mounts
        i = 0;
        while ((i<vfs_mounts->n) && (strcmp(path,vfs_mounts->mounts[i].path)==-1)) 
            i++;
        if (i==vfs_mounts->n) {
            vfs_mounts->mounts[vfs_mounts->n].path  = strdup(path);
            vfs_mounts->mounts[vfs_mounts->n].fs    = fs;
            vfs_mounts->n++;
        } else if (strcmp(path,vfs_mounts->mounts[i].path)==0) {
            return -EEXIST;
        } else {
            u32int j;
            for (j=vfs_mounts->n; j>=i; j--) {
                vfs_mounts->mounts[j+1].path    = vfs_mounts->mounts[j].path;
                vfs_mounts->mounts[j+1].fs      = vfs_mounts->mounts[j].fs;
            }
            vfs_mounts->mounts[i].path  = strdup(path);
            vfs_mounts->mounts[i].fs    = fs;
            vfs_mounts->n++;
        }

        // modify vnode
        node->flags |= VFS_MOUNTPOINT;
        to_mount->covered = node;
        node->ptr = to_mount;

    } else {
        return -1;
    }

    return 0;
}

void syscall_mount(char *src, char* dst, u32int flags, void *data)
{
}

void dump_vnode(vnode_t *node)
{
    if (!node) {
        scr_puts("vnode [null]\n");
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

