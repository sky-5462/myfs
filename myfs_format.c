#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FORMAT_INCLUDE
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
    
    struct myfs_super_block super;
    super.magic = MYFS_MAGIC;

    // Available size for data bitmap and data region
    uint32_t size = buf.st_size - MYFS_DATA_BMP_OFFSET * MYFS_BLOCK_SIZE;
    // Each group contains one data bitmap and several data blocks
    uint32_t groups = size / MYFS_DATA_SIZE_PER_GROUP;
    super.data_groups = groups;
    super.free_inodes = MYFS_INODE_COUNT - 2;
    super.free_data_blocks = groups * MYFS_DATA_REGION_BLOCKS_PER_GROUP;

    write(fd, &super, sizeof(super));

    struct myfs_inode root_inode = {
        .mode = S_IFDIR | 0744,
        .size = 0,
        .block_start = 0,
        .block_count = 0,
    };
    
    // skip inode 0
    lseek(fd, MYFS_INODE_REGION_OFFSET * MYFS_BLOCK_SIZE + MYFS_INODE_SIZE, SEEK_SET);
    write(fd, &root_inode, MYFS_INODE_SIZE);

    int bmp = 3;
    lseek(fd, MYFS_INODE_BMP_OFFSET * MYFS_BLOCK_SIZE, SEEK_SET);
    write(fd, &bmp, 1);
}