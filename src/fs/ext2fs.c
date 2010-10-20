#include "ext2fs.h"
#include "module.h"
#include "vfs.h"
#include "common.h"
#include "kheap.h"
#include "screen.h"

void    ext2fs_init(struct fs_driver *);
void    ext2fs_cleanup(struct fs_driver *);
fs_t*   ext2fs_createfs(struct fs_driver *drv, char *path, u32int flags, void *data);
void    ext2fs_removefs(struct fs_driver *, fs_t *fs);

vnode_t* ext2fs_get_root(fs_t *fs);
vnode_t* ext2fs_lookup(fs_t *fs, char *path);
void     ext2fs_drop_node(fs_t *fs, vnode_t *node);

s32int   ext2fs_open    (file_t *f);
s32int   ext2fs_close   (file_t *f);
s32int   ext2fs_read    (file_t *f, u32int offset, u32int sz, u8int *buffer);
s32int   ext2fs_write   (file_t *f, u32int offset, u32int sz, u8int *buffer);

s32int   ext2fs_subnodes(vnode_t *dir, vnode_t ***nodes);
s32int   ext2fs_mkdir   (vnode_t *dir, char *name, u32int flags);
s32int   ext2fs_mknod   (vnode_t *dir, char *name, u32int dev_id, u32int flags);
s32int   ext2fs_create  (vnode_t *dir, char *name, u32int flags);
s32int   ext2fs_rmdir   (vnode_t *dir, char *name);
s32int   ext2fs_rm      (vnode_t *dir, char *name);
s32int   ext2fs_rename  (vnode_t *dir, char *old_name, char *name);

struct fs_driver_operations ext2fs_drv_ops = {
    .init = &ext2fs_init,
    .cleanup = &ext2fs_cleanup,
    .createfs = &ext2fs_createfs,
    .removefs = &ext2fs_removefs,
};

struct fs_operations ext2fs_ops = {
    .get_root = &ext2fs_get_root,
    .lookup = &ext2fs_lookup,
    .drop_node = &ext2fs_drop_node,
};

struct file_operations ext2fs_fops = {
    .open = &ext2fs_open,
    .close = &ext2fs_close,
    .read = &ext2fs_read,
    .write = &ext2fs_write,
};

struct vnode_operations ext2fs_vops = {
    .subnodes = &ext2fs_subnodes,
    .mkdir = &ext2fs_mkdir,
    /*.mknod = &ext2fs_mknod,*/
    .create = &ext2fs_create,
    .rmdir = &ext2fs_rmdir,
    .rm = &ext2fs_rm,
    .rename = &ext2fs_rename,
};

struct fs_driver ext2fs_drv = {
    .name = "ext2fs",
    .fs_drv_ops = &ext2fs_drv_ops,
    .next = 0,
};

typedef struct {
    u32int nr_bgs;
    u32int blk_size;
    struct ext2_super_block sb;
    struct ext2_group_desc *bgs;
    file_t *devf;
} ext2fs_priv_t;

typedef struct {
    u32int p_ino;
    struct ext2_inode *inode;
} ext2fs_vnode_priv_t;

struct ext2_dir_entry dot_dentry = {
    .inode = 0,
    .name_len = 1,
    .rec_len = 12,
    .name = ".",
};

struct ext2_dir_entry dotdot_dentry = {
    .inode = 0,
    .name_len = 2,
    .rec_len = 12,
    .name = "..",
};

struct ext2_dir_entry empty_dentry = {
    .inode = 0,
    .name_len = 0,
    .rec_len = 8,
};

static u32int real_dentry_len(struct ext2_dir_entry *d)
{
    // 4-byte aligned
    return EXT2_DENTRY_BASE_LEN + ( (d->name_len+3) & ~3 );
}

static u32int get_dentry_len(u32int name_len)
{
    // 4-byte aligned
    return EXT2_DENTRY_BASE_LEN + ( (name_len+3) & ~3 );
}

static struct ext2_inode *get_inode(fs_t *fs, u32int ino)
{
    if (!fs || !ino)
        return 0;

    u32int n_bg, off_bg, off;

    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    struct ext2_super_block *sb = &p->sb;
    n_bg = (ino-1)/sb->s_inodes_per_group;
    // off_bg = (ino-1) % sb->s_inodes_per_group;
    off_bg = (ino-1) - n_bg*sb->s_inodes_per_group;

    struct ext2_inode *inode = (struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));
    memset(inode,0,sizeof(struct ext2_inode));
    off = p->bgs[n_bg].bg_inode_table*p->blk_size+off_bg*sizeof(struct ext2_inode); 
    /*
     *printk("n_bg:%d,off_bg:%d,offset:%d\n",n_bg,off_bg,off);
     *printk("bg_inode_table:%d\n",p->bgs[n_bg].bg_inode_table);
     */
    file_lseek(p->devf,off,SEEK_SET);
    file_read(p->devf,inode,sizeof(struct ext2_inode));

    return inode;
}

static s32int put_inode(fs_t *fs, struct ext2_inode *inode, u32int ino)
{
    if (!fs || !ino)
        return -1;

    u32int n_bg, off_bg, off;
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    struct ext2_super_block *sb = &p->sb;
    n_bg = (ino-1)/sb->s_inodes_per_group;
    // off_bg = (ino-1) % sb->s_inodes_per_group;
    off_bg = (ino-1) - n_bg*sb->s_inodes_per_group;

    off = p->bgs[n_bg].bg_inode_table*p->blk_size+off_bg*sizeof(struct ext2_inode); 
    /*
     *printk("n_bg:%d,off_bg:%d,offset:%d\n",n_bg,off_bg,off);
     *printk("bg_inode_table:%d\n",p->bgs[n_bg].bg_inode_table);
     */
    file_lseek(p->devf,off,SEEK_SET);
    file_write(p->devf,inode,sizeof(struct ext2_inode));

    return 0;
}

static s32int get_block(fs_t *fs, u8int *buf, u32int blk_no)
{
    if (!fs || !buf)
        return 0;

    s32int ret = 0;
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    file_lseek(p->devf,blk_no*p->blk_size,SEEK_SET);
    ret = file_read(p->devf,buf,p->blk_size);

    return ret;
}

static s32int put_block(fs_t *fs, u8int *buf, u32int blk_no)
{
    if (!fs || !buf)
        return 0;

    s32int ret = 0;
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    file_lseek(p->devf,blk_no*p->blk_size,SEEK_SET);
    ret = file_write(p->devf,buf,p->blk_size);

    return ret;
}

static u32int block_no(fs_t *fs,struct ext2_inode *inode, u32int off)
{
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    u32int np_blks = p->blk_size/4;
    u8int *buf;
    u32int ret, n_blk,off_blk;

    if (off<EXT2_NDIR_BLOCKS) {
        ret = inode->i_block[off];
    } else {
        buf = (u8int*)kmalloc(p->blk_size);
        if (off < EXT2_NDIR_BLOCKS+np_blks) {
            get_block(fs,buf,inode->i_block[EXT2_IND_BLOCK]);
            off -= EXT2_IND_BLOCK;
            ret = *(u32int*)(buf+4*off);
        } else if (off < EXT2_NDIR_BLOCKS+np_blks+np_blks*np_blks) {
            get_block(fs,buf,inode->i_block[EXT2_DIND_BLOCK]);
            off -= EXT2_IND_BLOCK+np_blks;
            n_blk = off/np_blks;
            off_blk = off - n_blk*np_blks;
            get_block(fs,buf,*(u32int*)(buf+4*n_blk));
            ret = *(u32int*)(buf+4*off_blk);
        } else if (off < EXT2_NDIR_BLOCKS+np_blks+np_blks*np_blks+np_blks*np_blks*np_blks) {
            get_block(fs,buf,inode->i_block[EXT2_TIND_BLOCK]);
            off -= EXT2_IND_BLOCK+np_blks+np_blks*np_blks;
            n_blk = off/(np_blks*np_blks);
            off_blk = off - n_blk*np_blks*np_blks;
            get_block(fs,buf,*(u32int*)(buf+4*n_blk));
            n_blk = off_blk/np_blks;
            off_blk = off_blk - n_blk*np_blks;
            get_block(fs,buf,*(u32int*)(buf+4*n_blk));
            ret = *(u32int*)(buf+4*off_blk);
        }
        kfree(buf);
    }
    return ret;
}

static s32int get_inode_blocks(fs_t *fs,u8int *buf,struct ext2_inode *inode,u32int start_blk,u32int n_blks)
{
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    u32int np_blks = p->blk_size/4;
    u8int *ind_buf = (u8int*)kmalloc(p->blk_size);
    u8int *dind_buf = (u8int*)kmalloc(p->blk_size);
    u8int *tind_buf = (u8int*)kmalloc(p->blk_size);
    u32int i,j,k,off,doff,toff;
    u32int size = n_blks*p->blk_size;

    for (i=start_blk;n_blks && (i<EXT2_NDIR_BLOCKS);i++) {
        get_block(fs,buf,inode->i_block[i]);
        buf += p->blk_size;
        n_blks--;
    }
    if (!n_blks)
        goto ok;

    get_block(fs,ind_buf,inode->i_block[EXT2_IND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        off = *(u32int*)(ind_buf+4*i);
        get_block(fs,buf,off);
        buf += p->blk_size;
        n_blks--;
    }
    if (!n_blks)
        goto ok;

    get_block(fs,dind_buf,inode->i_block[EXT2_DIND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        doff = *(u32int*)(dind_buf+4*i);
        get_block(fs,ind_buf,doff);
        for (j=0;n_blks && (j<np_blks);j++) {
            off = *(u32int*)(ind_buf+4*j);
            get_block(fs,buf,off);
            buf += p->blk_size;
            n_blks--;
        }
    }
    if (!n_blks)
        goto ok;

    get_block(fs,tind_buf,inode->i_block[EXT2_TIND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        toff = *(u32int*)(tind_buf+4*i);
        get_block(fs,dind_buf,toff);
        for (j=0;n_blks && (j<np_blks);j++) {
            doff = *(u32int*)(dind_buf+4*j);
            get_block(fs,ind_buf,doff);
            for (k=0;n_blks && (k<np_blks);k++) {
                off = *(u32int*)(ind_buf+4*k);
                get_block(fs,buf,off);
                buf += p->blk_size;
                n_blks--;
            }
        }
    }
ok:
    kfree(ind_buf);
    kfree(dind_buf);
    kfree(tind_buf);
    return size;
}

static s32int put_inode_blocks(fs_t *fs,u8int *buf,struct ext2_inode *inode,u32int start_blk,u32int n_blks)
{
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    u32int np_blks = p->blk_size/4;
    u8int *ind_buf = (u8int*)kmalloc(p->blk_size);
    u8int *dind_buf = (u8int*)kmalloc(p->blk_size);
    u8int *tind_buf = (u8int*)kmalloc(p->blk_size);
    u32int i,j,k,off,doff,toff;
    u32int size = n_blks*p->blk_size;

    for (i=start_blk;n_blks && (i<EXT2_NDIR_BLOCKS);i++) {
        put_block(fs,buf,inode->i_block[i]);
        buf += p->blk_size;
        n_blks--;
    }
    if (!n_blks)
        goto ok;

    get_block(fs,ind_buf,inode->i_block[EXT2_IND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        off = *(u32int*)(ind_buf+4*i);
        put_block(fs,buf,off);
        buf += p->blk_size;
        n_blks--;
    }
    if (!n_blks)
        goto ok;

    get_block(fs,dind_buf,inode->i_block[EXT2_DIND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        doff = *(u32int*)(dind_buf+4*i);
        get_block(fs,ind_buf,doff);
        for (j=0;n_blks && (j<np_blks);j++) {
            off = *(u32int*)(ind_buf+4*j);
            put_block(fs,buf,off);
            buf += p->blk_size;
            n_blks--;
        }
    }
    if (!n_blks)
        goto ok;

    get_block(fs,tind_buf,inode->i_block[EXT2_TIND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        toff = *(u32int*)(tind_buf+4*i);
        get_block(fs,dind_buf,toff);
        for (j=0;n_blks && (j<np_blks);j++) {
            doff = *(u32int*)(dind_buf+4*j);
            get_block(fs,ind_buf,doff);
            for (k=0;n_blks && (k<np_blks);k++) {
                off = *(u32int*)(ind_buf+4*k);
                put_block(fs,buf,off);
                buf += p->blk_size;
                n_blks--;
            }
        }
    }
ok:
    kfree(ind_buf);
    kfree(dind_buf);
    kfree(tind_buf);
    return size;
}

static vnode_t *alloc_vnode(fs_t *fs,struct ext2_inode *inode)
{
    if (!fs || !inode)
        return 0;

    vnode_t *node = (vnode_t*)kmalloc(sizeof(vnode_t));
    memset(node,0,sizeof(vnode_t));
    node->fs = fs;
    node->f_ops = &ext2fs_fops;
    node->v_ops = &ext2fs_vops;
    if (inode->i_mode & EXT2_S_IFDIR)
        node->flags |= VFS_DIRECTORY;
    else if (inode->i_mode & EXT2_S_IFREG) {
        node->flags |= VFS_FILE;
        node->length = inode->i_size;
    } else {
        node->flags |= VFS_UNKNOWN;
        // doesn't support other types yet
        printk("ext2fs(ERR): you're trying to get vnode from inode other than reg or dir.\n");
        kfree(node);
        return 0;
    }
    return node;
}

static u32int get_free_bit(u8int *buf, u32int sz)
{
    u32int i,j;
    for (i=0; i<sz; i++) {
        for (j=0; j<8; j++) {
            if ((buf[i] & (1<<j))==0)
                return i*8+j;
        }
    }
    return -1;
}

static s32int set_bit(u8int *buf, u32int off)
{
    if (off==-1)
        return -1;

    buf[off/8] |= 1 << (off & 7);

    return 0;
}

static s32int clear_bit(u8int *buf, u32int off)
{
    if (off==-1)
        return -1;

    buf[off/8] &= ~(1 << (off & ~7));

    return 0;
}

static u32int get_free_ino(fs_t *fs, u32int bg_hint)
{
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    if (p->sb.s_free_inodes_count == 0)
        return 0;
    u32int free_bg,i,ret = 0;
    u8int *buf = (u8int*)kmalloc(p->blk_size);


    if (p->bgs[bg_hint].bg_free_inodes_count)
        free_bg = bg_hint;
    else {
        free_bg = -1;
        for (i=0; i<p->nr_bgs; i++){
            if (p->bgs[i].bg_free_inodes_count) {
                free_bg = i;
                break;
            }
        }
        if (free_bg == -1)
            return 0;
    }

    get_block(fs,buf,p->bgs[free_bg].bg_inode_bitmap);
    ret = get_free_bit(buf,p->blk_size);
    printk("get_free_bit:%d\n",ret);
    set_bit(buf,ret);
    ret += 1+free_bg*p->sb.s_inodes_per_group;
    put_block(fs,buf,p->bgs[free_bg].bg_inode_bitmap);
    --p->bgs[bg_hint].bg_free_inodes_count;
    --p->sb.s_free_inodes_count;

    kfree(buf);
    return ret;
}

static u32int get_free_block(fs_t *fs, u32int bg_hint)
{
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    if (p->sb.s_free_blocks_count == 0)
        return 0;

    u32int free_bg,i,ret = 0;
    u8int *buf = (u8int*)kmalloc(p->blk_size);

    if (p->bgs[bg_hint].bg_free_blocks_count)
        free_bg = bg_hint;
    else {
        free_bg = -1;
        for (i=0; i<p->nr_bgs; i++){
            if (p->bgs[i].bg_free_blocks_count) {
                free_bg = i;
                break;
            }
        }
        if (free_bg == -1)
            return 0;
    }

    get_block(fs,buf,p->bgs[free_bg].bg_block_bitmap);
    ret = get_free_bit(buf,p->blk_size);
    set_bit(buf,ret);
    ret += free_bg*p->sb.s_blocks_per_group + p->sb.s_first_data_block;
    put_block(fs,buf,p->bgs[free_bg].bg_block_bitmap);
    --p->bgs[bg_hint].bg_free_blocks_count;
    --p->sb.s_free_blocks_count;

    kfree(buf);
    return ret;
}

static void expand_blocks(fs_t *fs,struct ext2_inode *inode,u32int new_blocks,u32int bg_hint)
{
    if (inode->i_blocks >= new_blocks)
        return;

    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    u32int np_blks = p->blk_size/4;
    u8int *ind_buf = (u8int*)kmalloc(p->blk_size);
    u8int *dind_buf = (u8int*)kmalloc(p->blk_size);
    u8int *tind_buf = (u8int*)kmalloc(p->blk_size);
    u32int i,j,k,off,doff,toff;
    u32int start_blk = inode->i_blocks;
    u32int n_blks = new_blocks - start_blk;

    inode->i_blocks = new_blocks;

    for (i=start_blk;n_blks && (i<EXT2_NDIR_BLOCKS);i++) {
        if (!inode->i_block[i])
            inode->i_block[i] = get_free_block(fs,bg_hint);
        n_blks--;
    }
    if (!n_blks)
        goto ok;

    if (!inode->i_block[EXT2_IND_BLOCK]) {
        inode->i_block[EXT2_IND_BLOCK] = get_free_block(fs,bg_hint);
        memset(ind_buf,0,p->blk_size);
    } else 
        get_block(fs,ind_buf,inode->i_block[EXT2_IND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        off = *(u32int*)(ind_buf+4*i);
        if (!off) {
            off = *(u32int*)(ind_buf+4*i) = get_free_block(fs,bg_hint);
        }
        n_blks--;
    }
    put_block(fs,ind_buf,inode->i_block[EXT2_IND_BLOCK]);
    if (!n_blks)
        goto ok;

    if (!inode->i_block[EXT2_DIND_BLOCK]) {
        inode->i_block[EXT2_DIND_BLOCK] = get_free_block(fs,bg_hint);
        memset(dind_buf,0,p->blk_size);
    } else 
        get_block(fs,dind_buf,inode->i_block[EXT2_DIND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        doff = *(u32int*)(dind_buf+4*i);
        if (!doff) {
            doff = *(u32int*)(dind_buf+4*i) = get_free_block(fs,bg_hint);
            memset(ind_buf,0,p->blk_size);
        } else 
            get_block(fs,ind_buf,doff);
        for (j=0;n_blks && (j<np_blks);j++) {
            off = *(u32int*)(ind_buf+4*j);
            if (!off) {
                off = *(u32int*)(ind_buf+4*j) = get_free_block(fs,bg_hint);
            }
            n_blks--;
        }
        put_block(fs,ind_buf,doff);
    }
    put_block(fs,dind_buf,inode->i_block[EXT2_DIND_BLOCK]);
    if (!n_blks)
        goto ok;

    if (!inode->i_block[EXT2_TIND_BLOCK]) {
        inode->i_block[EXT2_TIND_BLOCK] = get_free_block(fs,bg_hint);
        memset(tind_buf,0,p->blk_size);
    } else 
        get_block(fs,tind_buf,inode->i_block[EXT2_TIND_BLOCK]);
    for (i=0;n_blks && (i<np_blks);i++) {
        toff = *(u32int*)(tind_buf+4*i);
        if (!toff) {
            toff = *(u32int*)(tind_buf+4*i) = get_free_block(fs,bg_hint);
            memset(dind_buf,0,p->blk_size);
        } else 
            get_block(fs,dind_buf,toff);
        for (j=0;n_blks && (j<np_blks);j++) {
            doff = *(u32int*)(dind_buf+4*j);
            if (!doff) {
                doff = *(u32int*)(dind_buf+4*j) = get_free_block(fs,bg_hint);
                memset(ind_buf,0,p->blk_size);
            } else 
                get_block(fs,ind_buf,doff);
            for (k=0;n_blks && (k<np_blks);k++) {
                off = *(u32int*)(ind_buf+4*k);
                if (!off) {
                    off = *(u32int*)(ind_buf+4*k) = get_free_block(fs,bg_hint);
                }
                n_blks--;
            }
        }
        put_block(fs,dind_buf,toff);
    }
    put_block(fs,tind_buf,inode->i_block[EXT2_TIND_BLOCK]);
ok:
    kfree(ind_buf);
    kfree(dind_buf);
    kfree(tind_buf);
}

void module_ext2fs_init()
{
    register_fs_driver(&ext2fs_drv);
}

void module_ext2fs_cleanup()
{
    unregister_fs_driver(&ext2fs_drv);
}

void    ext2fs_init(struct fs_driver *fs_drv)
{
}

void    ext2fs_cleanup(struct fs_driver *fs_drv)
{
}

fs_t*   ext2fs_createfs(struct fs_driver *drv, char *path, u32int flags, void *data)
{
    fs_t *fs = (fs_t*)kmalloc(sizeof(fs_t));
    memset(fs,0,sizeof(fs_t));
    fs->fs_ops = &ext2fs_ops;
    fs->driver = drv;

    ext2fs_priv_t *ext2fs_priv = (ext2fs_priv_t*)kmalloc(sizeof(ext2fs_priv_t));
    memset(ext2fs_priv,0,sizeof(ext2fs_priv_t));
    ext2fs_priv->devf = file_open(path,0);
    if (!ext2fs_priv->devf)
        goto err;
    // loading superblock
    file_lseek(ext2fs_priv->devf,1024,SEEK_SET);
    file_read(ext2fs_priv->devf,&ext2fs_priv->sb,sizeof(struct ext2_super_block));
    u32int nr_blks = ext2fs_priv->sb.s_blocks_count-ext2fs_priv->sb.s_first_data_block;
    u32int blks_per_grp = ext2fs_priv->sb.s_blocks_per_group;
    ext2fs_priv->nr_bgs = (nr_blks+blks_per_grp-1)/blks_per_grp;
    u32int len = ext2fs_priv->nr_bgs*sizeof(struct ext2_group_desc);
    ext2fs_priv->bgs = (struct ext2_group_desc*)kmalloc(len);
    file_read(ext2fs_priv->devf,ext2fs_priv->bgs,len);
    ext2fs_priv->blk_size = 1024 << ext2fs_priv->sb.s_log_block_size;
    fs->priv = ext2fs_priv;

    return fs;
err:
    kfree(ext2fs_priv);
    kfree(fs);
    return 0;
}

void    ext2fs_removefs(struct fs_driver *fs_drv, fs_t *fs)
{
    kfree(fs);
}

vnode_t* ext2fs_get_root(fs_t *fs)
{
    if (!fs)
        return 0;

    struct ext2_inode *inode = get_inode(fs,EXT2_ROOT_INO);
    vnode_t *node = alloc_vnode(fs,inode);
    if (node) {
        ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)kmalloc(sizeof(ext2fs_vnode_priv_t));
        vp->p_ino = 0;
        vp->inode = inode;
        strcpy(node->name,"/");
        node->ino = EXT2_ROOT_INO;
        node->priv = vp;
        return node;
    } else 
        return 0;
}

vnode_t* ext2fs_lookup(fs_t *fs, char *path)
{
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    char **s = (char**)kmalloc(MAX_PATH_DEPS*sizeof(char*));
    memset(s,0,MAX_PATH_DEPS*sizeof(char*));
    u32int i,j,k,n,pos,p_ino,next_ino;
    struct ext2_inode *inode;
    struct ext2_dir_entry *d;
    u8int *buf = (u8int*)kmalloc(p->blk_size);

    if (strcmp(path,"/") == 0) {
        kfree(buf);
        return ext2fs_get_root(fs);
    }

    n = strbrk(s,path,"/");
    next_ino = EXT2_ROOT_INO;
    inode = get_inode(fs,next_ino);
    for (i=0; i<n; i++) {
        if (!inode || !(inode->i_mode & EXT2_S_IFDIR))
            goto err;
        u32int name_len = strlen(s[i]);

        next_ino = 0;
        pos = 0;
        for (j=0; j<inode->i_blocks; j++) {
            get_block(fs,buf,block_no(fs,inode,j));
            k = 0;
            do {
                d = (struct ext2_dir_entry*)(buf+k);
                if (d->rec_len == 0)
                    break;
                // used
                if (d->inode) {
                    if ((d->name_len == name_len) && (memcmp((u8int*)s[i],(u8int*)d->name,name_len)==0)) {
                        p_ino = next_ino;
                        next_ino = d->inode;
                        break;
                    }
                }
                k += d->rec_len;
            } while (k<p->blk_size);
            pos += k;
            if (next_ino || (pos >= inode->i_size))
                break;
        }

        kfree(inode);
        if (next_ino) {
            inode = get_inode(fs,next_ino);
        } else {
            inode = 0;
            break;
        }
    }

    if (inode) {
        vnode_t *node = alloc_vnode(fs,inode);
        if (node) {
            ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)kmalloc(sizeof(ext2fs_vnode_priv_t));
            memset(vp,0,sizeof(ext2fs_vnode_priv_t));
            vp->p_ino = p_ino;
            vp->inode = inode;
            node->priv = vp;
            strcpy(node->name,s[n-1]);
            node->ino = next_ino;
        } else {
            goto err;
        }
        
        kfree(buf);
        kfree(s);
        return node;
    }
err:
    kfree(buf);
    kfree(s);
    return 0;
}

void ext2fs_drop_node(fs_t *fs, vnode_t *node)
{
    ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)node->priv;
    kfree(vp->inode);
    kfree(vp);
    kfree(node);
    node = 0;
}

s32int   ext2fs_open    (file_t *f)
{
    return 0;
}

s32int   ext2fs_close   (file_t *f)
{
    return 0;
}

s32int   ext2fs_read    (file_t *f, u32int offset, u32int sz, u8int *buf)
{
    vnode_t *node = f->vnode;
    fs_t *fs = node->fs;
    ext2fs_priv_t *p = (ext2fs_priv_t*)(fs->priv);
    ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)node->priv;
    struct ext2_inode *inode = vp->inode;

    if (offset>=node->length)
        return 0;

    sz = MIN(node->length-offset,sz);

    u8int *blk_buf = (u8int*)kmalloc(p->blk_size);
    u32int size = sz;
    u32int len;
    u32int real_off = offset & ~(p->blk_size-1);

    // first block
    if (real_off<offset) {
        len = MIN(sz,real_off+p->blk_size-offset);
        get_block(fs,blk_buf,block_no(fs,inode,real_off/p->blk_size));
        memcpy(buf,blk_buf+offset-real_off,len);
        offset += len;
        buf += len;
        sz -= len;
    }
    if (!sz)
        goto ok;

    // middle aligned blocks
    if (sz/p->blk_size) {
        get_inode_blocks(fs,buf,inode,offset/p->blk_size,sz/p->blk_size);
        // len = sz - (sz % p->blk_size);
        len = sz & ~(p->blk_size-1);
        offset += len;
        buf += len;
        sz -= len;
    }
    if (!sz)
        goto ok;

    // last block
    get_block(fs,blk_buf,block_no(fs,inode,offset/p->blk_size));
    memcpy(buf,blk_buf,sz);
    offset += sz;
    buf += sz;

ok:
    kfree(blk_buf);      
    return size;
}

s32int   ext2fs_write   (file_t *f, u32int offset, u32int sz, u8int *buf)
{
    vnode_t *node = f->vnode;
    fs_t *fs = node->fs;
    ext2fs_priv_t *p = (ext2fs_priv_t*)(fs->priv);
    ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)node->priv;
    struct ext2_inode *inode = vp->inode;

    u8int *blk_buf = (u8int*)kmalloc(p->blk_size);
    u32int blk_no,len;
    s32int ret = sz, new_length = MAX(node->length,offset+sz);
    u32int n_bg, np_blks = p->blk_size/4;
    
    if (offset+sz > inode->i_blocks*p->blk_size) {
        // need to alloc free blocks
        u32int new_blocks = (offset+sz+p->blk_size-1)/p->blk_size;
        u32int extra_blocks = new_blocks/np_blks + new_blocks/(np_blks*np_blks) + 3;
        if (new_blocks + extra_blocks - inode->i_blocks > p->sb.s_free_blocks_count) {
            ret = -ENOSPC;
            goto cleanup;
        }
        n_bg = (node->ino-1)/p->sb.s_inodes_per_group;
        expand_blocks(fs,inode,new_blocks,n_bg);
    }

    // read first block and modify it, then write back, if offset not blk_size aligned
    if (offset & (p->blk_size-1)) {
        u32int real_offset = offset & ~(p->blk_size-1);
        blk_no = block_no(fs,inode,real_offset/p->blk_size);
        get_block(fs, blk_buf, blk_no);
        len = MIN(sz, real_offset+p->blk_size-offset);
        memcpy(blk_buf+offset-real_offset, buf, len);
        printk("blk_buf:%s\n",blk_buf);
        put_block(fs, blk_buf, blk_no);
        buf += len;
        offset += len;
        sz -= len;
    }

    // now offset is blk_size-aligned, write middle blocks
    if (sz >= p->blk_size) {
        put_inode_blocks(fs, buf, inode, offset/p->blk_size, sz/p->blk_size);
        len = sz & ~(p->blk_size-1);
        buf += len;
        offset += len;
        sz -= len;
    }

    // read last block, modify it, write back if needed
    if (sz>0) {
        blk_no = block_no(fs,inode,offset/p->blk_size);
        get_block(fs, blk_buf, blk_no);
        memcpy(blk_buf, buf, sz);
        put_block(fs, blk_buf, blk_no);
    }

    node->length = new_length;
    inode->i_size = new_length;
    put_inode(fs,inode,node->ino);

cleanup:
    kfree(blk_buf);
    return ret;
}

s32int   ext2fs_subnodes(vnode_t *dir, vnode_t ***nodes)
{
    s32int i,k,n = 0;
    struct ext2_inode *inode = ((ext2fs_vnode_priv_t*)dir->priv)->inode;
    struct ext2_dir_entry *d;
    vnode_list_t *l = 0;
    ext2fs_priv_t *p = (ext2fs_priv_t*)(dir->fs->priv);
    u8int *buf = (u8int*)kmalloc(p->blk_size);

    for (i=0; i<inode->i_blocks; i++) {
        get_block(dir->fs,buf,block_no(dir->fs,inode,i));
        k = 0;
        do {
            d = (struct ext2_dir_entry*)(buf+k);
            if (d->rec_len == 0)
                break;
            // used
            if (d->inode && d->name_len
                    && !((d->name_len==1) && (d->name[0]=='.')) 
                    && !((d->name_len==2)&&(d->name[0]=='.')&&(d->name[1]=='.'))) {

                struct ext2_inode *subinode = get_inode(dir->fs,d->inode);
                if ((inode->i_mode & EXT2_S_IFDIR) 
                        || (inode->i_mode & EXT2_S_IFREG)) {

                    vnode_t *subnode = (vnode_t*)kmalloc(sizeof(vnode_t));
                    memset(subnode,0,sizeof(vnode_t));
                    ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)kmalloc(sizeof(ext2fs_vnode_priv_t));
                    vp->p_ino = dir->ino;
                    vp->inode = subinode;
                    subnode->priv = vp;
                    memcpy((u8int*)subnode->name,(u8int*)d->name,d->name_len);
                    subnode->name[d->name_len] = 0;
                    if (inode->i_mode & EXT2_S_IFDIR)
                        subnode->flags |= VFS_DIRECTORY;
                    else if (inode->i_mode & EXT2_S_IFREG) {
                        subnode->flags |= VFS_FILE;
                        subnode->length = inode->i_size;
                    }
                    n++;

                    // insert to vnode_list_t *l
                    vnode_list_t *list_node = (vnode_list_t*)kmalloc(sizeof(vnode_list_t));
                    memset(list_node,0,sizeof(vnode_list_t));
                    list_node->node = subnode;

                    //    insert at front
                    if (l) {
                        list_node->next = l;
                        l->prev = list_node;
                    }
                    l = list_node;
                } else {
                    kfree(subinode);
                }
            }
            k += d->rec_len;
        } while (k<p->blk_size);
    } // for (i=0; i<inode->i_blocks; i++)

    kfree(buf);

    vnode_list_t *lp;

    // fillup ***nodes
    if (n>0 && nodes) {
        *nodes = (vnode_t**)kmalloc(sizeof(vnode_t*)*n);
        lp = l;
        for (i=0; i<n; i++) {
            (*nodes)[i] = lp->node;
            lp = lp->next;
        }
    }

    // free vnode_list_t *l
    while (l) {
        lp = l->next;
        kfree(l);
        l = lp;
    }

    return n;
}

s32int ext2fs_mkdir   (vnode_t *dir, char *name, u32int flags)
{
    fs_t *fs = dir->fs;
    ext2fs_priv_t *p = (ext2fs_priv_t*)fs->priv;
    ext2fs_vnode_priv_t *vp = (ext2fs_vnode_priv_t*)dir->priv;
    struct ext2_inode *inode;
    struct ext2_dir_entry *d;
    u8int *buf;
    u8int *blk_buf;
    u32int sub_ino, name_len, rec_len;
    u32int i,bg_hint;

    bg_hint = (dir->ino-1)/p->sb.s_inodes_per_group;

    // find a free ino
    sub_ino = get_free_ino(fs,bg_hint);
    if (!sub_ino)
        return -ENOSPC;

    inode = vp->inode;
    buf = (u8int*)kmalloc(inode->i_size);
    memset(buf,0,inode->i_size);

    get_inode_blocks(fs,buf,inode,0,inode->i_size/p->blk_size);
    name_len = strlen(name);
    rec_len = get_dentry_len(name_len);

    // first scan to check whether a child with same name exists
    i = 0;
    while (i<inode->i_size) {
        d = (struct ext2_dir_entry*)(buf+i);

        if (d->inode && (d->name_len==name_len) 
                && (memcmp((u8int*)d->name,(u8int*)name,name_len)==0)) {
            kfree(buf);
            return -EEXIST;
        }

        i += d->rec_len;
    }
    
    struct ext2_dir_entry *sub_d = (struct ext2_dir_entry*)kmalloc(sizeof(struct ext2_dir_entry));
    blk_buf = (u8int*)kmalloc(p->blk_size);

    memset(sub_d,0,sizeof(struct ext2_dir_entry));
    sub_d->inode = sub_ino;
    sub_d->name_len = name_len;
    sub_d->rec_len = rec_len;
    memcpy(sub_d->name,name,name_len);

    printk("ext2fs_mkdir:%s,sub_ino:%d\n",name,sub_ino);

    // second scan to find free space and insert
    i = 0;
    while (i<inode->i_size) {
        d = (struct ext2_dir_entry*)(buf+i);

        if (!d->inode && (d->rec_len>=rec_len)) {
            sub_d->rec_len = d->rec_len;
            memcpy(d,sub_d,rec_len);
            put_inode_blocks(fs,buf,inode,0,inode->i_blocks);
            goto ok;
        } else if (d->rec_len-real_dentry_len(d) >= rec_len) {
            d->rec_len -= rec_len;
            memcpy(buf+i+d->rec_len,sub_d,rec_len);
            put_inode_blocks(fs,buf,inode,0,inode->i_blocks);
            goto ok;
        }

        i += d->rec_len;
    }

    // can't insert ,need to expand block
    inode->i_size += p->blk_size;
    if (inode->i_size > p->blk_size*inode->i_blocks) {
        expand_blocks(fs,inode,inode->i_blocks+1,bg_hint);
    }

    memset(blk_buf,0,p->blk_size);
    sub_d->rec_len = p->blk_size;
    memcpy(blk_buf,sub_d,rec_len);
    put_inode_blocks(fs,blk_buf,inode,inode->i_blocks-1,1);
ok:
    put_inode(fs,inode,dir->ino);

    // create sub_inode
    u32int sub_bghint = (sub_ino-1)/p->sb.s_inodes_per_group;
    struct ext2_inode *sub_inode = (struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));
    memset(sub_inode,0,sizeof(struct ext2_inode));
    sub_inode->i_mode |= EXT2_S_IFDIR | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR
        | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH | EXT2_S_IXOTH;
    sub_inode->i_size = p->blk_size;
    sub_inode->i_blocks = 1;
    sub_inode->i_block[0] = get_free_block(fs,sub_bghint);
    u32int real_dot_len = real_dentry_len(&dot_dentry);
    u32int real_dotdot_len = real_dentry_len(&dotdot_dentry);
    u32int real_empty_len = get_dentry_len(0);
    dot_dentry.inode = sub_ino;
    dotdot_dentry.inode = dir->ino;
    empty_dentry.rec_len = p->blk_size - dot_dentry.rec_len - dotdot_dentry.rec_len;
    memcpy(blk_buf, &dot_dentry, real_dot_len);
    memcpy(blk_buf+real_dot_len, &dotdot_dentry, real_dotdot_len);
    memcpy(blk_buf+real_dot_len+real_dotdot_len, &empty_dentry, real_empty_len);
    put_block(fs,blk_buf,sub_inode->i_block[0]);
    put_inode(fs,sub_inode,sub_ino);
    
    kfree(sub_d);
    kfree(blk_buf);
    kfree(buf);

    return 0;
}

s32int ext2fs_mknod   (vnode_t *dir, char *name, u32int dev_id, u32int flags)
{
    return -EFAULT;
}

s32int ext2fs_create  (vnode_t *dir, char *name, u32int flags)
{
    return 0;
}

s32int   ext2fs_rmdir   (vnode_t *dir, char *name)
{
    return 0;
}

s32int   ext2fs_rm      (vnode_t *dir, char *name)
{
    return 0;
}

s32int ext2fs_rename (vnode_t *dir, char *old_name, char *name)
{
    return 0;
}

MODULE_INIT(module_ext2fs_init);
MODULE_CLEANUP(module_ext2fs_cleanup);

