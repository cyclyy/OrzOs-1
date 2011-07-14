#include "ext2fs.h"
#include "vfs.h"
#include "util.h"
#include "kmm.h"
#include "vmm.h"

s64int ext2fsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data);
s64int ext2fsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs);

s64int ext2fsRoot(struct FileSystem *fs);
s64int ext2fsOpen(struct FileSystem *fs, s64int id, struct VNode *node);
s64int ext2fsClose(struct VNode *node);
s64int ext2fsRead(struct VNode *node, u64int size, char *buffer);
s64int ext2fsSeek(struct VNode *node, s64int offset, s64int pos);
s64int ext2fsReaddir(struct VNode *node, u64int bufSize, char *buf);
s64int ext2fsFinddir(struct FileSystem *fs, s64int id, const char *name);
s64int ext2fsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info);

static struct FileSystemDriverOperation driverOps = {
    .mount = ext2fsMount,
    .unmount = ext2fsUnmount,
};

static struct FileSystemOperation fsOps = {
    .root = ext2fsRoot,
    .open = ext2fsOpen,
    .close = ext2fsClose,
    .read = ext2fsRead,
    .write = 0,
    .readdir = ext2fsReaddir,
    .finddir = ext2fsFinddir,
    .stat = ext2fsStat,
    //.seek = ext2fsSeek,
    .seek = vfsNopSeek,
} ;

struct FileSystemDriver ext2fsDriver = {
    .name = "ext2fs",
    .op = &driverOps,
};

struct Ext2FSData
{
    int nBlockGroups;
    int blockSize;
    struct VNode *node;
    struct ext2_super_block superBlock;
    struct ext2_group_desc *blockGroupDesc;
};

void ext2fs_Init()
{
    registerFileSystemDriver(&ext2fsDriver);
}

static int readINode(struct FileSystem *fs, int id, struct ext2_inode *inode)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)fs->priv;
    int groupIndex, inodeIndex, blockIndex, offset;

    groupIndex = (id-1) / fsData->superBlock.s_inodes_per_group;
    inodeIndex = (id-1) % fsData->superBlock.s_inodes_per_group;
    blockIndex = fsData->blockGroupDesc[groupIndex].bg_inode_table;
    offset = blockIndex * fsData->blockSize + inodeIndex * sizeof(struct ext2_inode);
    vfsSeek(fsData->node, offset, SEEK_SET);
    vfsRead(fsData->node, sizeof(struct ext2_inode), inode);
    return 0;
}

static int readBlock(struct FileSystem *fs, u32int index, void *buffer)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)fs->priv;

    vfsSeek(fsData->node, index * fsData->blockSize, SEEK_SET);
    vfsRead(fsData->node, fsData->blockSize, buffer);

    return 0;
}

static int readExt2Block(struct FileSystem *fs, struct ext2_inode *inode, u32int index, void *buffer)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)fs->priv;
    u32int nSubBlock = fsData->blockSize / 4;
    u32int blockIndex, indBlockIndex, dindBlockIndex;
    u32int *subBlock;

    if (index<EXT2_NDIR_BLOCKS) {
        blockIndex = inode->i_block[index];
    } else if (index < EXT2_NDIR_BLOCKS + nSubBlock) {
        subBlock = (u32int*)kMalloc(fsData->blockSize);
        readBlock(fs, inode->i_block[EXT2_IND_BLOCK], subBlock);
        index -= EXT2_NDIR_BLOCKS;
        blockIndex = subBlock[index];
        kFree(subBlock);
    } else if (index < EXT2_NDIR_BLOCKS + nSubBlock + nSubBlock * nSubBlock) {
        subBlock = (u32int*)kMalloc(fsData->blockSize);
        readBlock(fs, inode->i_block[EXT2_DIND_BLOCK], subBlock);
        index -= EXT2_NDIR_BLOCKS + nSubBlock;
        indBlockIndex = index / nSubBlock;
        readBlock(fs, subBlock[indBlockIndex], subBlock);
        blockIndex = subBlock[index % nSubBlock];
        kFree(subBlock);
    } else {
        DBG("not implemented!");
        for (;;);
    }
    readBlock(fs, blockIndex, buffer);
    return 0;
}

s64int ext2fsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data)
{
    struct VNode *node;
    struct Ext2FSData *fsData;
    s64int ret, len;

    fsData = (struct Ext2FSData*)kMalloc(sizeof(struct Ext2FSData));
    memset(fsData,0,sizeof(struct Ext2FSData));
    node = (struct VNode*)kMalloc(sizeof(struct VNode));
    memset(node,0,sizeof(struct VNode));
    fsData->node = node;
    ret = vfsOpen(source,0,node);
    if (ret < 0) {
        goto error;
    }
    vfsSeek(node,1024,SEEK_SET);
    vfsRead(node,sizeof(struct ext2_super_block),&fsData->superBlock);
    if (fsData->superBlock.s_magic != EXT2_MAGIC) {
        goto error;
    }
    fsData->nBlockGroups = (fsData->superBlock.s_blocks_count 
            - fsData->superBlock.s_first_data_block 
            + fsData->superBlock.s_blocks_per_group - 1)
        / (fsData->superBlock.s_blocks_per_group);
    fsData->blockSize = 1024 << fsData->superBlock.s_log_block_size;
    if (fsData->superBlock.s_log_block_size) {
        vfsSeek(node,fsData->blockSize,SEEK_SET);
    }
    len = fsData->nBlockGroups * sizeof(struct ext2_group_desc);
    fsData->blockGroupDesc = (struct ext2_group_desc*)kMalloc(len);
    vfsRead(node,len,fsData->blockGroupDesc);
    *fs = (struct FileSystem*)kMalloc(sizeof(struct FileSystem));
    (*fs)->driver = driver;
    (*fs)->op = &fsOps;
    (*fs)->priv = fsData;
    return 0;
error:
    kFree(node);
    kFree(fsData);
    *fs = 0;
    return -1;
}

s64int ext2fsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)fs->priv;
    vfsClose(fsData->node);
    kFree(fsData->node);
    kFree(fsData);
    return 0;
}

s64int ext2fsRoot(struct FileSystem *fs)
{
    return EXT2_ROOT_INO;
}

s64int ext2fsOpen(struct FileSystem *fs, s64int id, struct VNode *node)
{
    node->fs = fs;
    node->id = id;
    node->offset = 0;
    return 0;
}

s64int ext2fsClose(struct VNode *node)
{
    return 0;
}

s64int ext2fsRead(struct VNode *node, u64int bufSize, char *buffer)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)node->fs->priv;
    struct ext2_inode inode;
    struct ext2_dir_entry *dentry;
    char *blockBuffer;
    struct DirectoryRecord *drec;
    s64int ret, dsize, pos;
    u64int i, offset, off;

    readINode(node->fs, node->id, &inode);
    if (!(inode.i_mode & EXT2_S_IFDIR)) {
        return -1;
    }
    blockBuffer = (char*)kMalloc(fsData->blockSize);
    ret = 0;
    for (i=0; i<inode.i_blocks; i++) {
        readExt2Block(node->fs, &inode, i, blockBuffer);
        offset = off = pos = 0;
        dentry = (struct ext2_dir_entry*)(blockBuffer + offset);
        while ((offset < fsData->blockSize) && dentry->rec_len) {
            if (off == node->offset) {
                drec = (struct DirectoryRecord *)(buffer + pos);
                dsize = sizeof(struct DirectoryRecord) + dentry->name_len;
                if (pos + dsize <= bufSize) {
                    drec->size = dsize;
                    memcpy(drec->buffer, dentry->name, dentry->name_len);
                    drec->buffer[dentry->name_len] = 0;
                    pos += dsize;
                    node->offset++;
                    ret ++;
                } else {
                    break;
                }
            }
            off++;
            offset += dentry->rec_len;
            dentry = (struct ext2_dir_entry*)(blockBuffer + offset);
        }
    }
    kFree(blockBuffer);
    return ret;
}

s64int ext2fsReaddir(struct VNode *node, u64int bufSize, char *buf)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)node->fs->priv;
    struct ext2_inode inode;
    struct ext2_dir_entry *dentry;
    char *blockBuffer;
    struct DirectoryRecord *drec;
    s64int ret, dsize, pos;
    u64int i, offset, off;

    readINode(node->fs, node->id, &inode);
    if (!(inode.i_mode & EXT2_S_IFDIR)) {
        return -1;
    }
    blockBuffer = (char*)kMalloc(fsData->blockSize);
    ret = 0;
    for (i=0; i<inode.i_blocks; i++) {
        readExt2Block(node->fs, &inode, i, blockBuffer);
        offset = off = pos = 0;
        dentry = (struct ext2_dir_entry*)(blockBuffer + offset);
        while ((offset < fsData->blockSize) && dentry->rec_len) {
            if (off == node->offset) {
                drec = (struct DirectoryRecord *)(buf + pos);
                dsize = sizeof(struct DirectoryRecord) + dentry->name_len;
                if (pos + dsize <= bufSize) {
                    drec->size = dsize;
                    memcpy(drec->buffer, dentry->name, dentry->name_len);
                    drec->buffer[dentry->name_len] = 0;
                    pos += dsize;
                    node->offset++;
                    ret ++;
                } else {
                    break;
                }
            }
            off++;
            offset += dentry->rec_len;
            dentry = (struct ext2_dir_entry*)(blockBuffer + offset);
        }
    }
    kFree(blockBuffer);
    return ret;
}

s64int ext2fsFinddir(struct FileSystem *fs, s64int id, const char *name)
{
    struct Ext2FSData *fsData = (struct Ext2FSData*)fs->priv;
    struct ext2_inode inode;
    struct ext2_dir_entry *dentry;
    char *blockBuffer;
    s64int ret;
    u64int i, len, offset;

    readINode(fs, id, &inode);
    if (!(inode.i_mode & EXT2_S_IFDIR)) {
        return -1;
    }
    blockBuffer = (char*)kMalloc(fsData->blockSize);
    ret = 0;
    for (i=0; i<inode.i_blocks; i++) {
        readExt2Block(fs, &inode, i, blockBuffer);
        offset = 0;
        dentry = (struct ext2_dir_entry*)(blockBuffer + offset);
        while ((offset < fsData->blockSize) && dentry->rec_len) {
            len = strlen(name);
            if ((len == dentry->name_len) && (strncmp(dentry->name, name, len) == 0)) {
                ret = dentry->inode;
                goto finish;
            }
            offset += dentry->rec_len;
            dentry = (struct ext2_dir_entry*)(blockBuffer + offset);
        }
    }
finish:
    kFree(blockBuffer);
    return ret;
}

s64int ext2fsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info)
{
    return 0;
}

/*
s64int ext2fsSeek(struct VNode *node, s64int offset, s64int pos)
{
    return 0;
}
*/

