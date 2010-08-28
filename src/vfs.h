#ifndef VFS_H
#define VFS_H 

#include "common.h"
#include "file.h"

#define VFS_FILE 0x01
#define VFS_DIRECTORY    0x02
#define VFS_DEV_FILE  0x04
#define VFS_DEV_DIR 0x08
#define VFS_PIPE         0x16
#define VFS_SYMLINK      0x32
#define VFS_MOUNTPOINT   0x64 // Is the vnode an active mountpoint?
#define MAX_VFSMOUNTS    10
#define MAX_PATH_DEPS    20

typedef struct fs_struct fs_t;
typedef struct vnode_struct vnode_t;

struct file_operations {
    s32int   (*open)    (file_t *f);
    s32int   (*close)   (file_t *f);
    s32int   (*read)    (file_t *f, u32int offset, u32int sz, u8int *buffer);
    s32int   (*write)   (file_t *f, u32int offset, u32int sz, u8int *buffer);
};

struct vnode_operations {
    vnode_t* (*parent)  (vnode_t *node);
    s32int   (*subnodes)(vnode_t *dir, vnode_t ***nodes);
    s32int   (*mkdir)   (vnode_t *dir, char *name, u32int flags);
    s32int   (*mknod)   (vnode_t *dir, char *name, u32int dev_id, u32int flags);
    s32int   (*create)  (vnode_t *dir, char *name, u32int flags);
    s32int   (*rmdir)   (vnode_t *dir, char *name);
    s32int   (*rm)      (vnode_t *dir, char *name);
};

struct vnode_struct {
    char name[128]; //filename
    u32int length;

    u32int mask;    //permission
    u32int flags;   //filetype 
    u32int uid;     
    u32int gid;
    u32int dev_id;   // set by fs driver to identify the vnode, or (major, minor) if it's a device
    u32int ino; 

    struct file_operations  *f_ops;
    struct vnode_operations *v_ops;

    fs_t    *fs;    // the file system instance is resides in.
    vnode_t *covered;
    vnode_t *ptr;

    void *priv;
};

struct fs_operations {
    vnode_t* (*get_root)(fs_t *fs);
    void (*sync)(fs_t *fs);
    vnode_t* (*lookup)    (fs_t *fs, char *name);
};
    
struct fs_struct {
    struct fs_operations *fs_ops;
    struct fs_driver *driver;
    void *priv;
};

struct fs_driver_operations {
    void (*init)(struct fs_driver *);
    void (*cleanup)(struct fs_driver *);
    fs_t* (*createfs)(struct fs_driver *, vnode_t *dev, u32int flags, void *data);
    void (*removefs)(struct fs_driver *, fs_t *fs);
};

typedef struct fs_driver {
    char name[MAX_NAME_LEN];
    struct fs_driver_operations *fs_drv_ops;
    void *priv;
    struct fs_driver *next;
} fs_driver_t;

typedef struct {
    u32int n;
    struct {
        char *path;
        fs_t *fs;
    } mounts[MAX_VFSMOUNTS];
} vfsmount_t;

extern vnode_t      *vfs_root;
extern fs_driver_t  *fs_drivers;
extern vfsmount_t   *vfs_mounts;

void            init_vfs();

void            register_fs_driver(fs_driver_t *driver);
void            unregister_fs_driver(fs_driver_t *driver);
fs_driver_t*    get_fs_driver_byname(char *name);

char*           vfs_abs_path(char *rel_path);

// The difference with vnode->fs->*() and vfs_*() is:
//   vfs_*() works across file-systems, it handles mount points.
vnode_t*        vfs_lookup  (char *path);
s32int          vfs_subnodes(char *path, vnode_t ***nodes);
s32int          vfs_mkdir   (char *path, u32int flags);
s32int          vfs_mknod   (char *path, u32int dev_id, u32int flags);
s32int          vfs_create  (char *path, u32int flags);
s32int          vfs_rmdir   (char *path);
s32int          vfs_rm      (char *path);
s32int          vfs_mount   (char *path, fs_t *fs);
s32int          vfs_mount_root(fs_t *fs);

s32int          sys_mkdir   (char *path, u32int flags);
s32int          sys_mknod   (char *path, u32int dev_id, u32int flags);
s32int          sys_create  (char *path, u32int flags);
s32int          sys_rmdir   (char *path);
s32int          sys_rm      (char *path);

s32int          sys_getcwd  (char *buf,u32int size);
s32int          sys_chdir   (char *path);

void dump_vfs(char *path);

#endif /* VFS_H */
