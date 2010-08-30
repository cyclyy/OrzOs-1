#ifndef EXT2FS_H
#define EXT2FS_H 

#include "common.h"

#define EXT2_MAGIC              0xef53

/*
 * File system states
 */
#define EXT2_VALID_FS           0x0001  /* Unmounted cleanly */
#define EXT2_ERROR_FS           0x0002  /* Errors detected */

/*
 * Behaviour when detecting errors
 */
#define EXT2_ERRORS_CONTINUE        1   /* Continue execution */
#define EXT2_ERRORS_RO          2   /* Remount fs read-only */
#define EXT2_ERRORS_PANIC       3   /* Panic */
#define EXT2_ERRORS_DEFAULT     EXT2_ERRORS_CONTINUE

/*
 * Codes for operating systems
 */
#define EXT2_OS_LINUX       0
#define EXT2_OS_HURD        1
#define EXT2_OS_MASIX       2
#define EXT2_OS_FREEBSD     3
#define EXT2_OS_LITES       4

/*
 * Revision levels
 */
#define EXT2_GOOD_OLD_REV   0   /* The good old (original) format */
#define EXT2_DYNAMIC_REV    1   /* V2 format w/ dynamic inode sizes */


#define EXT2_FEATURE_COMPAT_DIR_PREALLOC    0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES   0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL     0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR        0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO      0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX       0x0020
#define EXT2_FEATURE_COMPAT_ANY         0xffffffff

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x0004
#define EXT2_FEATURE_RO_COMPAT_ANY      0xffffffff

#define EXT2_FEATURE_INCOMPAT_COMPRESSION   0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE      0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER       0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV   0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG       0x0010
#define EXT2_FEATURE_INCOMPAT_ANY       0xffffffff

#define EXT2_S_IFSOCK   0xc000
#define EXT2_S_IFLNK    0xa000
#define EXT2_S_IFREG    0x8000
#define EXT2_S_IFBLK    0x6000
#define EXT2_S_IFDIR    0x4000
#define EXT2_S_IFCHR    0x2000
#define EXT2_S_IFIFO    0x1000

/*
 * Special inode numbers
 */
#define EXT2_BAD_INO         1  /* Bad blocks inode */
#define EXT2_ROOT_INO        2  /* Root inode */
#define EXT2_BOOT_LOADER_INO     5  /* Boot loader inode */
#define EXT2_UNDEL_DIR_INO   6  /* Undelete directory inode */

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS        12
#define EXT2_IND_BLOCK          EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK         (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK         (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS           (EXT2_TIND_BLOCK + 1)

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
    EXT2_FT_UNKNOWN     = 0,
    EXT2_FT_REG_FILE    = 1,
    EXT2_FT_DIR     = 2,
    EXT2_FT_CHRDEV      = 3,
    EXT2_FT_BLKDEV      = 4,
    EXT2_FT_FIFO        = 5,
    EXT2_FT_SOCK        = 6,
    EXT2_FT_SYMLINK     = 7,
    EXT2_FT_MAX
};


/*
 * Structure of the super block
 */
struct ext2_super_block {
    u32int  s_inodes_count;     /* Inodes count */
    u32int  s_blocks_count;     /* Blocks count */
    u32int  s_r_blocks_count;   /* Reserved blocks count */
    u32int  s_free_blocks_count;    /* Free blocks count */
    u32int  s_free_inodes_count;    /* Free inodes count */
    u32int  s_first_data_block; /* First Data Block */
    u32int  s_log_block_size;   /* Block size */
    u32int  s_log_frag_size;    /* Fragment size */
    u32int  s_blocks_per_group; /* # Blocks per group */
    u32int  s_frags_per_group;  /* # Fragments per group */
    u32int  s_inodes_per_group; /* # Inodes per group */
    u32int  s_mtime;        /* Mount time */
    u32int  s_wtime;        /* Write time */
    u16int  s_mnt_count;        /* Mount count */
    u16int  s_max_mnt_count;    /* Maximal mount count */
    u16int  s_magic;        /* Magic signature */
    u16int  s_state;        /* File system state */
    u16int  s_errors;       /* Behaviour when detecting errors */
    u16int  s_minor_rev_level;  /* minor revision level */
    u32int  s_lastcheck;        /* time of last check */
    u32int  s_checkinterval;    /* max. time between checks */
    u32int  s_creator_os;       /* OS */
    u32int  s_rev_level;        /* Revision level */
    u16int  s_def_resuid;       /* Default uid for reserved blocks */
    u16int  s_def_resgid;       /* Default gid for reserved blocks */
    /*
     * These fields are for EXT2_DYNAMIC_REV superblocks only.
     *
     * Note: the difference between the compatible feature set and
     * the incompatible feature set is that if there is a bit set
     * in the incompatible feature set that the kernel doesn't
     * know about, it should refuse to mount the filesystem.
     * 
     * e2fsck's requirements are more strict; if it doesn't know
     * about a feature in either the compatible or incompatible
     * feature set, it must abort and not try to meddle with
     * things it doesn't understand...
     */
    u32int  s_first_ino;        /* First non-reserved inode */
    u16int   s_inode_size;      /* size of inode structure */
    u16int  s_block_group_nr;   /* block group # of this superblock */
    u32int  s_feature_compat;   /* compatible feature set */
    u32int  s_feature_incompat;     /* incompatible feature set */
    u32int  s_feature_ro_compat;    /* readonly-compatible feature set */
    u8int    s_uuid[16];     /* 128-bit uuid for volume */
    char    s_volume_name[16];  /* volume name */
    char    s_last_mounted[64];     /* directory where last mounted */
    u32int  s_algorithm_usage_bitmap; /* For compression */
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT2_COMPAT_PREALLOC flag is on.
     */
    u8int    s_prealloc_blocks;  /* Nr of blocks to try to preallocate*/
    u8int    s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
    u16int   s_padding1;
    /*
     * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
     */
    u8int    s_journal_uuid[16]; /* uuid of journal superblock */
    u32int   s_journal_inum;     /* inode number of journal file */
    u32int   s_journal_dev;      /* device number of journal file */
    u32int   s_last_orphan;      /* start of list of inodes to delete */
    u32int   s_hash_seed[4];     /* HTREE hash seed */
    u8int    s_def_hash_version; /* Default hash version to use */
    u8int    s_reserved_char_pad;
    u16int   s_reserved_word_pad;
    u32int  s_default_mount_opts;
    u32int  s_first_meta_bg;    /* First metablock block group */
    u32int   s_reserved[190];    /* Padding to the end of the block */
};

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
    u32int  bg_block_bitmap;        /* Blocks bitmap block */
    u32int  bg_inode_bitmap;        /* Inodes bitmap block */
    u32int  bg_inode_table;     /* Inodes table block */
    u16int  bg_free_blocks_count;   /* Free blocks count */
    u16int  bg_free_inodes_count;   /* Free inodes count */
    u16int  bg_used_dirs_count; /* Directories count */
    u16int  bg_pad;
    u32int  bg_reserved[3];
};

struct ext2_inode {
    u16int  i_mode;     /* File mode */
    u16int  i_uid;      /* Low 16 bits of Owner Uid */
    u32int  i_size;     /* Size in bytes */
    u32int  i_atime;    /* Access time */
    u32int  i_ctime;    /* Creation time */
    u32int  i_mtime;    /* Modification time */
    u32int  i_dtime;    /* Deletion Time */
    u16int  i_gid;      /* Low 16 bits of Group Id */
    u16int  i_links_count;  /* Links count */
    u32int  i_blocks;   /* Blocks count */
    u32int  i_flags;    /* File flags */
    union {
        struct {
            u32int  l_i_reserved1;
        } linux1;
        struct {
            u32int  h_i_translator;
        } hurd1;
        struct {
            u32int  m_i_reserved1;
        } masix1;
    } osd1;             /* OS dependent 1 */
    u32int  i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
    u32int  i_generation;   /* File version (for NFS) */
    u32int  i_file_acl; /* File ACL */
    u32int  i_dir_acl;  /* Directory ACL */
    u32int  i_faddr;    /* Fragment address */
    union {
        struct {
            u8int    l_i_frag;   /* Fragment number */
            u8int    l_i_fsize;  /* Fragment size */
            u16int   i_pad1;
            u16int  l_i_uid_high;   /* these 2 fields    */
            u16int  l_i_gid_high;   /* were reserved2[0] */
            u32int   l_i_reserved2;
        } linux2;
        struct {
            u8int    h_i_frag;   /* Fragment number */
            u8int    h_i_fsize;  /* Fragment size */
            u16int  h_i_mode_high;
            u16int  h_i_uid_high;
            u16int  h_i_gid_high;
            u32int  h_i_author;
        } hurd2;
        struct {
            u8int    m_i_frag;   /* Fragment number */
            u8int    m_i_fsize;  /* Fragment size */
            u16int   m_pad1;
            u32int   m_i_reserved2[2];
        } masix2;
    } osd2;             /* OS dependent 2 */
};

/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry {
	u32int	inode;			/* Inode number */
	u16int	rec_len;		/* Directory entry length */
	u8int	name_len;		/* Name length */
	u8int	file_type;
	char	name[EXT2_NAME_LEN];	/* File name */
};
#endif /* EXT2FS_H */
