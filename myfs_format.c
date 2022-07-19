#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FORMAT_INCLUDE
#include "myfs.h"

// myfs_format [path] [write_test_dir?]
int main(int argc, char** argv) {
    if (argc != 2 || argc != 3)
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
    super.free_data_blocks = groups * MYFS_DATA_REGION_BLOCKS_PER_GROUP - 1;
    write(fd, &super, sizeof(super));

    struct myfs_inode root_inode = {
        .mode = S_IFDIR | 0755,
        .nlink = 2,
        .size = 2 * MYFS_DENTRY_SIZE,
        .block_start = 0,
    };
    
    // skip inode 0
    lseek(fd, MYFS_INODE_REGION_OFFSET * MYFS_BLOCK_SIZE + MYFS_INODE_SIZE, SEEK_SET);
    write(fd, &root_inode, MYFS_INODE_SIZE);

    uint8_t inode_bmp = 0x3;
    lseek(fd, MYFS_INODE_BMP_OFFSET * MYFS_BLOCK_SIZE, SEEK_SET);
    write(fd, &inode_bmp, 1);

    uint8_t data_bmp = 0x1;
    lseek(fd, MYFS_DATA_BMP_OFFSET * MYFS_BLOCK_SIZE, SEEK_SET);
    write(fd, &data_bmp, 1);

    struct myfs_dentry dentry = {
        .ino = 1,
        .name_len = 1,
        .name = "."
    };
    lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE, SEEK_SET);
    write(fd, &dentry, MYFS_DENTRY_SIZE);

    dentry.ino = 0;
    dentry.name_len = 2;
    strcpy(dentry.name, "..");
    lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE + MYFS_DENTRY_SIZE, SEEK_SET);
    write(fd, &dentry, MYFS_DENTRY_SIZE);

    if (argc == 3 && argv[2][0] == '1') {
        /* test dir tree:
            / ----- test_dir/
                |
                --- test_file
        */
        super.free_inodes = MYFS_INODE_COUNT - 4;
        super.free_data_blocks = groups * MYFS_DATA_REGION_BLOCKS_PER_GROUP - 3;
        lseek(fd, 0, SEEK_SET);
        write(fd, &super, sizeof(super));

        inode_bmp = 0xF;
        lseek(fd, MYFS_INODE_BMP_OFFSET * MYFS_BLOCK_SIZE, SEEK_SET);
        write(fd, &inode_bmp, 1);

        data_bmp = 0x7;
        lseek(fd, MYFS_DATA_BMP_OFFSET * MYFS_BLOCK_SIZE, SEEK_SET);
        write(fd, &data_bmp, 1);

        root_inode.size = 4 * MYFS_DENTRY_SIZE;
        lseek(fd, MYFS_INODE_REGION_OFFSET * MYFS_BLOCK_SIZE + MYFS_INODE_SIZE, SEEK_SET);
        write(fd, &root_inode, MYFS_INODE_SIZE);

        struct myfs_inode dir_inode = {
            .mode = S_IFDIR | 0755,
            .nlink = 2,
            .size = 2 * MYFS_DENTRY_SIZE,
            .block_start = 1,
        };
        lseek(fd, MYFS_INODE_REGION_OFFSET * MYFS_BLOCK_SIZE + 2 * MYFS_INODE_SIZE, SEEK_SET);
        write(fd, &dir_inode, MYFS_INODE_SIZE);

        struct myfs_inode file_inode = {
            .mode = S_IFREG | 0644,
            .nlink = 1,
            .size = 5,
            .block_start = 2,
        };
        lseek(fd, MYFS_INODE_REGION_OFFSET * MYFS_BLOCK_SIZE + 3 * MYFS_INODE_SIZE, SEEK_SET);
        write(fd, &file_inode, MYFS_INODE_SIZE);

        dentry.ino = 2;
        dentry.name_len = 9;
        strcpy(dentry.name, "test_dir");
        lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE + 3 * MYFS_DENTRY_SIZE, SEEK_SET);
        write(fd, &dentry, MYFS_DENTRY_SIZE);

        dentry.ino = 3;
        dentry.name_len = 8;
        strcpy(dentry.name, "test_file");
        lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE + 2 * MYFS_DENTRY_SIZE, SEEK_SET);
        write(fd, &dentry, MYFS_DENTRY_SIZE);

        dentry.ino = 2;
        dentry.name_len = 1;
        strcpy(dentry.name, ".");
        lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE + MYFS_BLOCK_SIZE + 0 * MYFS_DENTRY_SIZE, SEEK_SET);
        write(fd, &dentry, MYFS_DENTRY_SIZE);

        dentry.ino = 1;
        dentry.name_len = 2;
        strcpy(dentry.name, "..");
        lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE + MYFS_BLOCK_SIZE + 1 * MYFS_DENTRY_SIZE, SEEK_SET);
        write(fd, &dentry, MYFS_DENTRY_SIZE);

        lseek(fd, MYFS_DATA_REGION_OFFSET(groups) * MYFS_BLOCK_SIZE + 2 * MYFS_BLOCK_SIZE, SEEK_SET);
        write(fd, "Hello", 5);
    } 
}