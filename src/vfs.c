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

vnode_t* vfs_lookup(char *path)
{
    if (!vfs_mounts)
        return 0;

    u32int i;
    for (i=vfs_mounts->n-1; i>=0; i--) {
        char *s = strstr(path,vfs_mounts->mounts[i].path);
        if (s==path) {
            fs_t *fs = vfs_mounts->mounts[i].fs;
            if (strcmp(path, vfs_mounts->mounts[i].path)==0)
                return fs->fs_ops->get_root(fs);

            s = path+strlen(vfs_mounts->mounts[i].path);
            if (strcmp(vfs_mounts->mounts[i].path,"/")==0) 
                s--;
            if (fs->fs_ops->lookup)
                return fs->fs_ops->lookup(fs, s);
            else 
                return 0;
        }
    }
    return 0;
}

s32int          vfs_subnodes(char *path, vnode_t ***nodes)
{
    s32int ret;
    vnode_t *dir = vfs_lookup(path);
    if (dir && dir->v_ops && dir->v_ops->subnodes)
        ret = dir->v_ops->subnodes(dir,nodes);
    else 
        ret = -EFAULT;
    return ret;
}

s32int          vfs_mkdir   (char *path, u32int flags)
{
    s32int ret;
    char *dir_name;
    char *base_name;
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    printk("mkdir:dirname %s basename %s dir %p\n",dir_name,base_name,dir);
    if (dir && dir->v_ops && dir->v_ops->mkdir)
        ret = dir->v_ops->mkdir(dir,base_name,flags);
    else 
        ret = -EFAULT;
    kfree(dir_name);
    kfree(base_name);
    return ret;
}

s32int          vfs_mknod   (char *path, u32int dev_id, u32int flags)
{
    s32int ret;
    char *dir_name;
    char *base_name;
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    if (dir && dir->v_ops && dir->v_ops->mknod)
        ret = dir->v_ops->mknod(dir,base_name,dev_id,flags);
    else 
        ret = -EFAULT;
    kfree(dir_name);
    kfree(base_name);
    return ret;
}

s32int          vfs_create  (char *path, u32int flags)
{
    s32int ret;
    char *dir_name;
    char *base_name;
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    if (dir && dir->v_ops && dir->v_ops->create)
        ret = dir->v_ops->create(dir,base_name,flags);
    else 
        ret = -EFAULT;
    kfree(dir_name);
    kfree(base_name);
    return ret;
}

s32int          vfs_rmdir   (char *path)
{
    s32int ret;
    char *dir_name;
    char *base_name;
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    if (dir && dir->v_ops && dir->v_ops->rmdir)
        ret = dir->v_ops->rmdir(dir,base_name);
    else 
        ret = -EFAULT;
    kfree(dir_name);
    kfree(base_name);
    return ret;
}

s32int          vfs_rm      (char *path)
{
    s32int ret;
    char *dir_name;
    char *base_name;
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    if (dir && dir->v_ops && dir->v_ops->rm)
        ret = dir->v_ops->rm(dir,base_name);
    else 
        ret = -EFAULT;
    kfree(dir_name);
    kfree(base_name);
    return ret;
}

s32int vfs_mount_root(fs_t *fs)
{
    if (!fs || !vfs_mounts)
        return -1;

    // root must be mounted first
    if (vfs_mounts->n != 0)
        return -1;

    vfs_root = fs->fs_ops->get_root(fs);

    if (vfs_root) {
        vfs_mounts->mounts[0].path = strdup("/");
        vfs_mounts->mounts[0].fs   = fs;
        vfs_mounts->n++;
    } else {
        return -1;
    }

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

    vnode_t *node     = vfs_lookup(path);
    vnode_t *to_mount = fs->fs_ops->get_root(fs);
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

void dump_vfs(char *path)
{
    vnode_t *node = vfs_lookup(path);
    if (node) {
        if (node->flags & VFS_FILE)
            printk("%s FILE\n",path);
        if (node->flags & VFS_DIRECTORY) {
            printk("%s DIR\n",path);
            vnode_t **sub_nodes = 0;
            u32int nsubs = vfs_subnodes(path,&sub_nodes);
            u32int i;
            for (i=0; i<nsubs; i++) {
                char *new_path = (char*)kmalloc(strlen(path)+strlen(sub_nodes[i]->name)+2);
                sprintf(new_path, "%s/%s", path, sub_nodes[i]->name);
                dump_vfs(new_path);
                kfree(new_path);
            }
            kfree(sub_nodes);
        }
        if (node->flags & VFS_DEV_FILE) {
            printk("%s DEV\n",path);
        }
    }
}

