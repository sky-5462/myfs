#ifndef FORMAT_INCLUDE
    #include <linux/buffer_head.h>
    #include <linux/types.h>
#else
    #include <stdint.h>
#endif

#define MYFS_MAGIC 0x5242087
#define MYFS_BLOCK_SHIFT 12
#define MYFS_BLOCK_SIZE (1 << MYFS_BLOCK_SHIFT)
#define MYFS_MAX_SIZE ((uint32_t)(-1))

struct myfs_super_block {
    uint32_t magic;
    uint32_t data_groups;
    uint32_t free_inodes;
    uint32_t free_data_blocks;
};

struct myfs_sb_info {
    struct buffer_head* s_sbh;
    struct myfs_super_block* s_es;
};

struct myfs_inode {
    uint32_t mode;
    uint32_t nlink;
    uint32_t size;
    uint32_t block_start;
};

#define MYFS_ROOT_INO 1

struct myfs_dentry {
    uint32_t ino;
    uint32_t name_len;
    char name[256 - 2 * sizeof(uint32_t)];  // without '\0'
};

#define MYFS_SUPER_BLOCK_SIZE (sizeof(struct myfs_super_block))
#define MYFS_INODE_BMP_SIZE MYFS_BLOCK_SIZE
#define MYFS_INODE_SIZE (sizeof(struct myfs_inode))
#define MYFS_INODE_COUNT (MYFS_INODE_BMP_SIZE * 8)
#define MYFS_INODE_REGION_SIZE (MYFS_INODE_COUNT * MYFS_INODE_SIZE)
#define MYFS_INODE_REGION_BLOCKS (MYFS_INODE_REGION_SIZE / MYFS_BLOCK_SIZE)
#define MYFS_INODE_COUNT_PER_BLOCK (MYFS_BLOCK_SIZE / MYFS_INODE_SIZE)
#define MYFS_DATA_BMP_SIZE MYFS_BLOCK_SIZE
#define MYFS_DATA_REGION_BLOCKS_PER_GROUP (MYFS_DATA_BMP_SIZE * 8)
#define MYFS_DATA_BLOCKS_PER_GROUP (1 + MYFS_DATA_REGION_BLOCKS_PER_GROUP)
#define MYFS_DATA_SIZE_PER_GROUP (MYFS_DATA_BLOCKS_PER_GROUP * MYFS_BLOCK_SIZE)
#define MYFS_DENTRY_SIZE (sizeof(struct myfs_dentry))
#define MYFS_DENTRY_COUNT_PER_BLOCK (MYFS_BLOCK_SIZE / MYFS_DENTRY_SIZE)

#define MYFS_INODE_BMP_OFFSET 1
#define MYFS_INODE_REGION_OFFSET 2
#define MYFS_DATA_BMP_OFFSET (MYFS_INODE_REGION_OFFSET + MYFS_INODE_REGION_BLOCKS)
#define MYFS_DATA_REGION_OFFSET (MYFS_INODE_REGION_OFFSET + MYFS_INODE_REGION_BLOCKS)


/* Format:
      0          1              2.. 
    super | inode_bitmap | inode_region | data_bitmap | data_region |
*/

