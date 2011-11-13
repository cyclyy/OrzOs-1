#include "ramfs.h"
#include "vfs.h"
#include "kmm.h"

void    ramfs_init(struct fs_driver *);
void    ramfs_cleanup(struct fs_driver *);
fs_t*   ramfs_createfs(struct fs_driver *drv, char *path, u32int flags, void *data);
void    ramfs_removefs(struct fs_driver *, fs_t *fs);

vnode_t* ramfs_get_root(fs_t *fs);
vnode_t* ramfs_lookup(fs_t *fs, char *path);
void     ramfs_drop_node(fs_t *fs, vnode_t *node);

s32int   ramfs_open    (file_t *f);
s32int   ramfs_close   (file_t *f);
s32int   ramfs_read    (file_t *f, u32int offset, u32int sz, u8int *buffer);
s32int   ramfs_write   (file_t *f, u32int offset, u32int sz, u8int *buffer);

s32int   ramfs_subnodes(vnode_t *dir, vnode_t ***nodes);
s32int   ramfs_mkdir   (vnode_t *dir, char *name, u32int flags);
s32int   ramfs_mknod   (vnode_t *dir, char *name, u32int dev_id, u32int flags);
s32int   ramfs_create  (vnode_t *dir, char *name, u32int flags);
s32int   ramfs_rmdir   (vnode_t *dir, char *name);
s32int   ramfs_rm      (vnode_t *dir, char *name);
s32int   ramfs_rename  (vnode_t *dir, char *old_name, char *name);

static struct FileSystemOperation ramfsOp = {
    .open = &ramfsOpen,
    .close = &ramfsClose,
    .read = &ramfsRead,
    .write = &ramfsWrite,
};

struct fs_operations ramfs_ops = {
    .get_root = &ramfs_get_root,
    .lookup = &ramfs_lookup,
    .drop_node = &ramfs_drop_node,
};

struct fs_driver_operations ramfs_drv_ops = {
    .init = &ramfs_init,
    .cleanup = &ramfs_cleanup,
    .createfs = &ramfs_createfs,
    .removefs = &ramfs_removefs,
};

typedef struct {
    vnode_t *root;
} ramfs_priv_t;

typedef struct {
    u32int nchildren;
    vnode_list_t *children;
    u8int *buffer;
} ramfs_vnode_priv_t;

struct fs_driver ramfs_drv = {
    .name = "ramfs",
    .fs_drv_ops = &ramfs_drv_ops,
    .next = 0,
};

static vnode_t *find_child(vnode_t *dir, char *name)
{
    ramfs_vnode_priv_t *dir_priv;
    dir_priv = (ramfs_vnode_priv_t*)dir->priv;
    vnode_list_t *p = dir_priv->children;
    while (p) {
        if (strcmp(p->node->name,name)==0)
            return p->node;
        p = p->next;
    }

    return 0;
}

static void add_child(vnode_t *dir, vnode_t *node)
{
    ramfs_vnode_priv_t *dir_priv;
    dir_priv = (ramfs_vnode_priv_t*)dir->priv;
    vnode_list_t *list_node = (vnode_list_t*)kmalloc(sizeof(vnode_list_t));
    list_node->node = node;
    list_node->next = dir_priv->children;
    list_node->prev = 0;
    if (dir_priv->children)
        dir_priv->children->prev = list_node;
    dir_priv->children = list_node;
    dir_priv->nchildren++;
}

static void remove_child(vnode_t *dir, vnode_t *node)
{
    ramfs_vnode_priv_t *dir_priv;
    dir_priv = (ramfs_vnode_priv_t*)dir->priv;
    vnode_list_t *p = dir_priv->children;
    while (p) {
        if (p->node==node) {
            if (p->prev)
                p->prev->next = p->next;
            if (p->next)
                p->next->prev = p->prev;
            if (p == dir_priv->children)
                dir_priv->children = p->next;
            dir_priv->nchildren--;
            return;
        }
        p = p->next;
    }
}

void module_ramfs_init()
{
    register_fs_driver(&ramfs_drv);
}

void module_ramfs_cleanup()
{
    unregister_fs_driver(&ramfs_drv);
}

void    ramfs_init(struct fs_driver *fs_drv)
{
}
    
void    ramfs_cleanup(struct fs_driver *fs_drv)
{
}

fs_t*   ramfs_createfs(struct fs_driver *drv, char *path, u32int flags, void *data)
{
    fs_t *fs = (fs_t*)kmalloc(sizeof(fs_t));
    memset(fs,0,sizeof(fs_t));
    fs->fs_ops = &ramfs_ops;
    fs->driver = drv;

    vnode_t *root = (vnode_t*)kmalloc(sizeof(vnode_t));
    memset(root,0,sizeof(root));
    strcpy(root->name,"ramfs_root");
    root->flags = VFS_DIRECTORY;
    root->f_ops = &ramfs_fops;
    root->v_ops = &ramfs_vops;
    root->fs = fs;
    ramfs_vnode_priv_t *vnode_priv = (ramfs_vnode_priv_t*)kmalloc(sizeof(ramfs_vnode_priv_t));
    memset(vnode_priv,0,sizeof(ramfs_vnode_priv_t));
    root->priv = vnode_priv;

    ramfs_priv_t *ramfs_priv = (ramfs_priv_t*)kmalloc(sizeof(ramfs_priv_t));
    memset(ramfs_priv,0,sizeof(ramfs_priv_t));
    ramfs_priv->root = root;
    fs->priv = ramfs_priv;

    return fs;
}

void    ramfs_removefs(struct fs_driver *fs_drv, fs_t *fs)
{
    kfree(fs);
}

vnode_t* ramfs_get_root(fs_t *fs)
{
    ramfs_priv_t *ramfs_priv = (ramfs_priv_t*)fs->priv;
    if (ramfs_priv)
        return ramfs_priv->root;

    return 0;
}

vnode_t* ramfs_lookup(fs_t *fs, char *path)
{
    char **s = (char**)kmalloc(MAX_PATH_DEPS*sizeof(char*));
    memset(s,0,MAX_PATH_DEPS*sizeof(char*));

    u32int i,j,n, nsubnodes, found;
    n = strbrk(s,path,"/");
    vnode_t *node = fs->fs_ops->get_root(fs);
    vnode_t **subnodes=0;
    for (i=0; i<n; i++) {
        if (!(node->flags & VFS_DIRECTORY))
            return 0;
        nsubnodes = node->v_ops->subnodes(node,&subnodes);
        found = 0;
        for (j=0; j<nsubnodes; j++)
            if (strcmp(subnodes[j]->name,s[i])==0) {
                node = subnodes[j];
                found = 1;
                break;
            }
        kfree(subnodes);
        subnodes = 0;
        if (!found)
            return 0;
    }
    return node;
}

void ramfs_drop_node (fs_t *fs, vnode_t *node)
{
    // doesn't drop it
}

s32int   ramfs_open    (file_t *f)
{
    return 0;
}

s32int   ramfs_close   (file_t *f)
{
    return 0;
}

s32int   ramfs_read    (file_t *f, u32int offset, u32int sz, u8int *buf)
{
    if (!f->vnode || !(f->vnode->flags&VFS_FILE) || !f->vnode->priv)
        return -1;

    if (offset>=f->vnode->length)
        return 0;

    ramfs_vnode_priv_t *vnode_priv = (ramfs_vnode_priv_t*)f->vnode->priv;
    u32int n = MIN(sz, f->vnode->length-offset);
    memcpy(buf,vnode_priv->buffer+offset,n);
    return n;
}

s32int   ramfs_write   (file_t *f, u32int offset, u32int sz, u8int *buf)
{
    if (!f->vnode || !(f->vnode->flags&VFS_FILE) || !f->vnode->priv)
        return -1;

    if (sz==0)
        return 0;

    ramfs_vnode_priv_t *vnode_priv = (ramfs_vnode_priv_t*)f->vnode->priv;

    if (offset+sz <= f->vnode->length) {
        memcpy(vnode_priv->buffer+offset,buf,sz);
    } else {
        u8int *new_buf = (u8int*)kmalloc(offset+sz);
        memcpy(new_buf,vnode_priv->buffer,f->vnode->length);
        memcpy(new_buf+offset,buf,sz);
        kfree(vnode_priv->buffer);
        vnode_priv->buffer = new_buf;
        f->vnode->length = offset+sz;
    }
    return sz;
}

s32int   ramfs_subnodes(vnode_t *dir, vnode_t ***nodes)
{
    if (!(dir->flags & VFS_DIRECTORY))
        return 0;

    ramfs_vnode_priv_t *dir_priv = (ramfs_vnode_priv_t*)(dir->priv);
    if (dir_priv->nchildren==0)
        return 0;
    if (nodes==0)
        return dir_priv->nchildren;
    *nodes = (vnode_t**)kmalloc(sizeof(vnode_t*)*dir_priv->nchildren);
    s32int n=0;
    vnode_list_t *p = dir_priv->children;
    while (p) {
        (*nodes)[n++] = p->node;
        p = p->next;
    }
    return dir_priv->nchildren;
}

s32int ramfs_mkdir   (vnode_t *dir, char *name, u32int flags)
{
    if (!dir || !(dir->flags&VFS_DIRECTORY)) {
        return -EFAULT;
    }

    if (find_child(dir, name)) {
        return -EEXIST;
    }

    vnode_t *node = (vnode_t*)kmalloc(sizeof(vnode_t));
    memset(node,0,sizeof(vnode_t));
    strcpy(node->name,name);
    node->flags = VFS_DIRECTORY;
    node->v_ops = &ramfs_vops;
    node->fs = dir->fs;
    ramfs_vnode_priv_t *vnode_priv = (ramfs_vnode_priv_t*)kmalloc(sizeof(ramfs_vnode_priv_t));
    memset(vnode_priv,0,sizeof(ramfs_vnode_priv_t));
    node->priv = vnode_priv;

    add_child(dir,node);

    return 0;
}

s32int ramfs_mknod   (vnode_t *dir, char *name, u32int dev_id, u32int flags)
{
    if (!dir || !(dir->flags&VFS_DIRECTORY)) {
        return -EFAULT;
    }

    if (find_child(dir, name)) {
        return -EEXIST;
    }

    vnode_t *node = (vnode_t*)kmalloc(sizeof(vnode_t));
    memset(node,0,sizeof(vnode_t));
    strcpy(node->name,name);
    node->flags = VFS_DEV_FILE;
    node->dev_id = dev_id;
    node->fs = dir->fs;
    ramfs_vnode_priv_t *vnode_priv = (ramfs_vnode_priv_t*)kmalloc(sizeof(ramfs_vnode_priv_t));
    memset(vnode_priv,0,sizeof(ramfs_vnode_priv_t));
    node->priv = vnode_priv;

    add_child(dir,node);

    return 0;
}

s32int ramfs_create  (vnode_t *dir, char *name, u32int flags)
{
    if (!dir || !(dir->flags&VFS_DIRECTORY)) {
        return -EFAULT;
    }

    if (find_child(dir, name)) {
        return -EEXIST;
    }

    vnode_t *node = (vnode_t*)kmalloc(sizeof(vnode_t));
    memset(node,0,sizeof(vnode_t));
    strcpy(node->name,name);
    node->flags = VFS_FILE;
    node->f_ops = &ramfs_fops;
    node->fs = dir->fs;
    ramfs_vnode_priv_t *vnode_priv = (ramfs_vnode_priv_t*)kmalloc(sizeof(ramfs_vnode_priv_t));
    memset(vnode_priv,0,sizeof(ramfs_vnode_priv_t));
    node->priv = vnode_priv;

    add_child(dir,node);

    return 0;
}

s32int   ramfs_rmdir   (vnode_t *dir, char *name)
{
    vnode_t *node = find_child(dir,name);

    if (!node)
        return -ENOENT;
        
    if (!(node->flags & VFS_DIRECTORY))
        return -ENOTDIR;

    if (node->v_ops->subnodes(node,0))
        return -EEXIST;

    remove_child(dir,node);

    return 0;
}

s32int   ramfs_rm      (vnode_t *dir, char *name)
{
    vnode_t *node = find_child(dir,name);

    if (!node)
        return -ENOENT;
        
    if (!(node->flags & VFS_FILE) && !(node->flags & VFS_DEV_FILE))
        return -ENOTDIR;

    remove_child(dir,node);

    return 0;
}

s32int ramfs_rename (vnode_t *dir, char *old_name, char *name)
{
    vnode_t *node = find_child(dir,old_name);

    if (!node)
        return -ENOENT;

    strcpy(node->name,name);
        
    return 0;
}

MODULE_INIT(module_ramfs_init);
MODULE_CLEANUP(module_ramfs_cleanup);
