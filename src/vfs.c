#include "vfs.h"
#include "kheap.h"
#include "screen.h"
#include "errno.h"
#include "dev.h"
#include "file.h"
#include "task.h"

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

char* vfs_abs_path(char *p)
{
    if (!p || (strlen(p)==0))
        return 0;

    char *path;
    if (*p != '/') {
        u32int len = strlen(p) + strlen(current_task->work_path) + 2;
        path = (char*)kmalloc(len);
        /*memset(path,0,len);*/
        sprintf(path, "%s/%s", current_task->work_path, p);
    } else 
        path = strdup(p);

    char **s = (char**)kmalloc(MAX_PATH_DEPS*sizeof(char*));
    memset(s,0,MAX_PATH_DEPS*sizeof(char*));

    u32int i,n,err = 0;
    u32int pos = 0;
    // assume abs_path is no longer than original path
    char *ret = (char*)kmalloc(strlen(path)+1);
    char *tmpstr;
    memset(ret,0,strlen(path)+1);
    n = strbrk(s,path,"/");
    if (n==0) {
        strcpy(ret,"/");
    } else {
        for (i=0; i<n; i++) {
            if (strcmp(s[i],".") == 0)
                continue;
            if (strcmp(s[i],"..") == 0) {
                tmpstr = strrchr(ret,'/');
                if (!tmpstr) {
                    err = 1;
                    break;
                }
                pos = tmpstr - ret;
                ret[pos] = 0;
                continue;
            }
            ret[pos] = '/';
            strcpy(ret+pos+1,s[i]);
            pos += 1+strlen(s[i]);
            ret[pos] = 0;
        }
        if ((err==0) && (pos==0)) {
            strcpy(ret,"/");
        }
    }
    kfree(path);
    kfree(s);
    if (err) {
        kfree(ret);
        return 0;
    } else 
        return ret;
}

vnode_t* vfs_lookup(char *path)
{
    if (!vfs_mounts || !path || (*path==0))
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
    vnode_t *node;
    if ((node = vfs_lookup(path))) {
        if (node->fs->fs_ops && node->fs->fs_ops->drop_node)
            node->fs->fs_ops->drop_node(node->fs,node);
        return -EEXIST;
    }
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    if (dir && dir->v_ops && dir->v_ops->mkdir)
        ret = dir->v_ops->mkdir(dir,base_name,flags);
    else 
        ret = -EFAULT;
    if (dir->fs->fs_ops && dir->fs->fs_ops->drop_node)
        dir->fs->fs_ops->drop_node(dir->fs,dir);
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
    printk("vfs_mknod %s %p\n",path,dev_id);
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
    if (vfs_subnodes(dir_name,0)) {
        ret = -EFAULT;
        goto cleanup;
    }
    if (dir && dir->v_ops && dir->v_ops->rmdir)
        ret = dir->v_ops->rmdir(dir,base_name);
    else 
        ret = -EFAULT;
cleanup:
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

s32int vfs_rename  (char *path, char *name)
{
    s32int ret;
    char *dir_name;
    char *base_name;
    dir_name = dirname(path);
    base_name = basename(path);
    vnode_t *dir = vfs_lookup(dir_name);
    if (dir && dir->v_ops && dir->v_ops->rm)
        ret = dir->v_ops->rename(dir,base_name,name);
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
    /*vnode_t *to_mount = fs->fs_ops->get_root(fs);*/
    u32int i;

    if (!node)
        return  -EFAULT;

    if ( (node->flags & VFS_DIRECTORY) && !(node->flags & VFS_MOUNTPOINT) ) {
        // add to vfs_mounts
        i = 0;
        while ((i<vfs_mounts->n) && (strcmp(path,vfs_mounts->mounts[i].path)==1)) {
            i++;
        }
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
        /*node->flags |= VFS_MOUNTPOINT;*/
        /*to_mount->covered = node;*/
        /*node->ptr = to_mount;*/

    } else {
        return -1;
    }

    return 0;
}

void syscall_mount(char *src, char* dst, u32int flags, void *data)
{
}

void vfs_dump(char *path)
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
                vfs_dump(new_path);
                kfree(new_path);
            }
            kfree(sub_nodes);
        }
        if (node->flags & VFS_DEV_FILE) {
            printk("%s DEV\n",path);
        }
    }
}

s32int          sys_mkdir   (char *path, u32int flags)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int ret; 
    u32int n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        kfree(kpath);
        return -EFAULT;
    }

    char *abs_path = vfs_abs_path(kpath);
    ret = vfs_mkdir(abs_path, flags);
    kfree(abs_path);
    kfree(kpath);
    return ret;
}

s32int          sys_mknod   (char *path, u32int dev_id, u32int flags)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int ret; 
    u32int n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        kfree(kpath);
        return -EFAULT;
    }

    char *abs_path = vfs_abs_path(kpath);
    ret = vfs_mknod(abs_path, dev_id, flags);
    kfree(abs_path);
    kfree(kpath);
    return ret;
}

s32int          sys_create  (char *path, u32int flags)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int ret; 
    u32int n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        kfree(kpath);
        return -EFAULT;
    }

    char *abs_path = vfs_abs_path(kpath);
    ret = vfs_create(abs_path, flags);
    kfree(abs_path);
    kfree(kpath);
    return ret;
}

s32int          sys_rmdir   (char *path)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int ret; 
    u32int n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        kfree(kpath);
        return -EFAULT;
    }

    char *abs_path = vfs_abs_path(kpath);
    ret = vfs_rmdir(abs_path);
    kfree(abs_path);
    kfree(kpath);
    return ret;
}

s32int          sys_rm      (char *path)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int ret; 
    u32int n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        kfree(kpath);
        return -EFAULT;
    }

    char *abs_path = vfs_abs_path(kpath);
    ret = vfs_rm(abs_path);
    kfree(abs_path);
    kfree(kpath);
    return ret;
}

s32int          sys_getcwd  (char *buf, u32int size)
{
    s32int n, copied;
    n = strlen(current_task->work_path) + 1;
    if (size < n)
        return -EFAULT;
    copied = copy_to_user(buf,current_task->work_path,n);
    if (copied < n)
        return -EFAULT;

    return 0;
}

s32int          sys_rename  (char *path, char *name)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    char *kname = (char*)kmalloc(MAX_NAME_LEN);
    memset(kname,0,MAX_NAME_LEN);
    s32int ret; 
    u32int n;
    n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        ret = -EFAULT;
        goto cleanup;
    }

    n = copy_from_user(kname, name, MAX_NAME_LEN);
    
    if (n==0) {
        ret = -EFAULT;
        goto cleanup;
    }
    char *abs_path = vfs_abs_path(kpath);
    ret = vfs_rename(abs_path,name);
    kfree(abs_path);
cleanup:
    kfree(kpath);
    kfree(kname);
    return ret;
}

s32int          sys_chdir   (char *path)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int ret; 
    copy_from_user(kpath, path, MAX_PATH_LEN);
    kpath[MAX_PATH_LEN-1] = 0;
    char *abs_path = vfs_abs_path(kpath);
    vnode_t *node = vfs_lookup(abs_path);
    if (node && (node->flags & VFS_DIRECTORY)) {
        strcpy(current_task->work_path,abs_path);
        ret = 0;
    } else 
        ret = -EFAULT;

    kfree(kpath);
    kfree(abs_path);
    return ret;
}

