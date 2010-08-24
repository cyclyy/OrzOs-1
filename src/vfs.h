#ifndef VFS_H
#define VFS_H 

#include "common.h"

#define VFS_FILE 0x01
#define VFS_DIRECTORY    0x02
#define VFS_DEV_FILE  0x04
#define VFS_DEV_DIR 0x08
#define VFS_PIPE         0x16
#define VFS_SYMLINK      0x32
#define VFS_MOUNTPOINT   0x64 // Is the vnode an active mountpoint?

struct vnode;
struct fs;
struct fs_driver;

typedef struct vnode {
    char name[128]; //filename
    u32int length;

    u32int mask;    //permission
    u32int flags;   //filetype 
    u32int uid;     
    u32int gid;
    u32int id;   // set by fs driver to identify the vnode, or (major, minor) if it's a device
    u32int impl; 

    u32int (*read)(struct vnode *vnode, u32int offset, u32int sz, u8int *buffer);
    u32int (*write)(struct vnode *vnode, u32int offset, u32int sz, u8int *buffer);
    void (*open)(struct vnode *vnode);
    void (*close)(struct vnode *vnode);

    struct vnode* (*readdir)(struct vnode *vnode, u32int i);
    struct vnode* (*finddir)(struct vnode *vnode, char *name);

    struct vnode *covered;
    struct vnode *ptr;
} vnode_t;
    
typedef struct fs {
    vnode_t* (*get_root)(struct fs *fs);
    void (*sync)(struct fs *fs);
    struct fs_driver *driver;
} fs_t;

typedef struct fs_driver {
    char name[MAX_NAME_LEN];

    void (*init)(struct fs_driver *);
    void (*cleanup)(struct fs_driver *);

    fs_t* (*createfs)(struct fs_driver *, vnode_t *dev, u32int flags, void *data);
    void (*removefs)(struct fs_driver *, fs_t *fs);

    struct fs_driver *next;
} fs_driver_t;

extern vnode_t *vfs_root;
extern fs_driver_t *fs_drivers;

void init_vfs();

void register_fs_driver(fs_driver_t *driver);
void unregister_fs_driver(fs_driver_t *driver);
fs_driver_t* get_fs_driver_byname(char *name);

u32int vfs_read(vnode_t *vnode, u32int offset, u32int sz, u8int *buffer);
u32int vfs_write(vnode_t *vnode, u32int offset, u32int sz, u8int *buffer);
void vfs_open(vnode_t *vnode);
void vfs_close(vnode_t *vnode);
vnode_t* vfs_readdir(vnode_t *vnode, u32int i);
vnode_t* vfs_finddir(struct vnode *vnode, char *name);
vnode_t *vfs_lookup(vnode_t *vnode, char *path);

void vfs_mount(vnode_t *node, fs_t *fs);

void dump_vnode(vnode_t *);

#endif /* VFS_H */
