#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "myfs.h"

// myfs_format [path]
int main(int argc, char** argv) {
    if (argc != 2)
        return -1;

    int fd = open(argv[1], O_RDWR);
    if (fd == -1)
        return -2;
    
    struct stat buf;
    if (fstat(fd, &buf) == -1)
        return -3;
    
    int size = buf.st_size;
    struct myfs_super_block super = {
        .magic = MYFS_MAGIC,
        .block_size = MYFS_BLOCK_SIZE,
        .block_shift = MYFS_BLOCK_SHIFT,
        .max_size = MYFS_MAX_SIZE,
        .inode_bitmap_offset = 1,
        .root_inode = 1,
    };
    // minus super_block size
    size -= MYFS_BLOCK_SIZE;
    // minus inode bitmap block
    size -= MYFS_BLOCK_SIZE;
    // minus inode blocks
    int inode_entry_counts = 8 * MYFS_BLOCK_SIZE;
    int inode_total_size = inode_entry_counts / MYFS_INODE_SIZE;
    int inode_blocks = inode_total_size / MYFS_BLOCK_SIZE;
    super.inode_region_offset = 2;
    super.inode_region_blocks = inode_blocks;
    size -= inode_total_size;

    int curr_block = 2 + inode_blocks;
    int data_blocks_per_bitmap_block = 8 * MYFS_BLOCK_SIZE;
    int blocks_per_group = data_blocks_per_bitmap_block + MYFS_BLOCK_SIZE;
    int groups = size / (blocks_per_group * MYFS_BLOCK_SIZE);
    int data_bitmap_blocks = groups;
    super.data_bitmap_offset = curr_block;
    super.data_bitmap_blocks = groups;
    super.data_region_offset = curr_block + groups;
    super.data_region_blocks = groups * data_blocks_per_bitmap_block;

    write(fd, &super, sizeof(super));

    struct myfs_inode root_inode = {
        .mode = S_IFDIR | 0744,
        .size = 0,
        .block_start = 0,
        .block_count = 0,
    };
    
    // skip inode 0
    lseek(fd, super.inode_region_offset * MYFS_BLOCK_SIZE + MYFS_INODE_SIZE, SEEK_SET);
    write(fd, &root_inode, MYFS_INODE_SIZE);

    int bmp = 2;
    lseek(fd, super.inode_bitmap_offset * MYFS_BLOCK_SIZE, SEEK_SET);
    write(fd, &bmp, 1);
}