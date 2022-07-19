#include <linux/buffer_head.h>
#include <linux/types.h>

#include "myfs.h"

static struct myfs_inode* myfs_get_inode(struct super_block *sb, ino_t ino,
                                         struct buffer_head **p) {
    struct buffer_head * bh;
    struct myfs_super_block* sbi;
    uint32_t block_index;
    uint32_t block_offset;

    *p = NULL;
    if (ino < MYFS_ROOT_INO || ino > MYFS_INODE_COUNT)
        goto Einval;

    sbi = (struct myfs_super_block*)(sb->s_fs_info);
    block_index = ino / MYFS_INODE_COUNT_PER_BLOCK + MYFS_INODE_REGION_OFFSET;
    block_offset = ino % MYFS_INODE_COUNT_PER_BLOCK;
    if (!(bh = sb_bread(sb, block_index)))
        goto Eio;

    *p = bh;
    return (struct myfs_inode *) (bh->b_data + block_offset * MYFS_INODE_SIZE);

Einval:
    printk(KERN_ERR "[myfs_get_inode] bad inode number: %lu\n", (unsigned long)ino);
    return ERR_PTR(-EINVAL);
Eio:
    printk(KERN_ERR "[myfs_get_inode] unable to read inode block - inode=%lu, block=%u\n", (unsigned long)ino, block_index);
    return ERR_PTR(-EIO);
}

struct inode* myfs_iget(struct super_block *sb, unsigned long ino) {
    struct buffer_head * bh = NULL;
    struct myfs_inode *raw_inode;
    struct inode *inode;
    long ret = -EIO;

    inode = iget_locked(sb, ino);
    if (!inode)
        return ERR_PTR(-ENOMEM);
    if (!(inode->i_state & I_NEW))
        return inode;

    raw_inode = myfs_get_inode(sb, ino, &bh);
    if (IS_ERR(raw_inode)) {
        ret = PTR_ERR(raw_inode);
        goto bad_inode;
    }

    inode->i_mode = raw_inode->mode;
    set_nlink(inode, raw_inode->nlink);
    inode->i_size = raw_inode->size;
    if (inode->i_nlink == 0 && inode->i_mode == 0) {
        /* this inode is deleted */
        ret = -ESTALE;
        goto bad_inode;
    }
    inode->i_blocks = (raw_inode->size + MYFS_BLOCK_SIZE - 1) / MYFS_BLOCK_SIZE;

    if (S_ISREG(inode->i_mode)) {
        inode->i_op = &myfs_file_inode_operations;
        inode->i_fop = &myfs_file_operations;
        inode->i_mapping->a_ops = &myfs_aops;
    } else if (S_ISDIR(inode->i_mode)) {
        inode->i_op = &myfs_dir_inode_operations;
        inode->i_fop = &myfs_dir_operations;
        inode->i_mapping->a_ops = &myfs_aops;
    } else if (S_ISLNK(inode->i_mode)) {
        inode->i_op = &myfs_symlink_inode_operations;
        inode_nohighmem(inode);
        inode->i_mapping->a_ops = &myfs_aops;
    }
    brelse (bh);
    unlock_new_inode(inode);
    return inode;
    
bad_inode:
    brelse(bh);
    iget_failed(inode);
    return ERR_PTR(ret);
}

const struct address_space_operations myfs_aops = {
};