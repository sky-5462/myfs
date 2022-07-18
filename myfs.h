#define MYFS_MAGIC 0x5242087
#define MYFS_BLOCK_SHIFT 12
#define MYFS_BLOCK_SIZE (1 << MYFS_BLOCK_SHIFT)
#define MYFS_MAX_SIZE ((uint32_t)(-1))

struct myfs_super_block {
    uint32_t magic;
    uint32_t block_size;
    uint32_t block_shift;
    uint32_t max_size;
    uint32_t inode_bitmap_offset;
    uint32_t inode_region_offset;
    uint32_t inode_region_blocks;
    uint32_t data_bitmap_offset;
    uint32_t data_bitmap_blocks;
    uint32_t data_region_offset;
    uint32_t data_region_blocks;
    uint32_t root_inode;
};

struct myfs_inode {
    uint32_t mode;
    uint32_t size;
    uint32_t block_start;
    uint32_t block_count;
};

#define MYFS_INODE_SIZE (sizeof(struct myfs_inode))