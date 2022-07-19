#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "myfs.h"

static const struct inode_operations myfs_file_inode_operations = {
};

static const struct file_operations myfs_file_operations = {
};

static const struct inode_operations myfs_dir_inode_operations = {
};

static const struct file_operations myfs_dir_operations = {
};

static const struct inode_operations myfs_symlink_inode_operations = {
};

static const struct address_space_operations myfs_aops = {
};


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

static void myfs_put_super(struct super_block* sb) {
    struct myfs_sb_info* sbi = (struct myfs_sb_info*)(sb->s_fs_info);
    brelse(sbi->s_sbh);
    kfree(sbi);
    sb->s_fs_info = NULL;
}

static const struct super_operations myfs_sops = {
    .put_super = myfs_put_super,
};

static int myfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct myfs_sb_info* sbi;
    struct myfs_super_block* es;
    struct buffer_head * bh;
    struct inode* inode;
    int ret = -ENOMEM;

    sbi = kzalloc(sizeof(struct myfs_sb_info), GFP_KERNEL);
    if (!sbi)
        return ret;

    ret = -EINVAL;
    sb->s_fs_info = sbi;
    if (sb_set_blocksize(sb, MYFS_BLOCK_SIZE) == 0)
        goto failed_sbi;

    if (!(bh = sb_bread(sb, 0))) {
        printk(KERN_ERR "error: unable to read superblock\n");
        goto failed_sbi;
    }

    es = (struct myfs_super_block*)(bh->b_data);
    sbi->s_es = es;
    sbi->s_sbh = bh;
    sb->s_magic = es->magic;
    if (sb->s_magic != MYFS_MAGIC) {
        printk(KERN_ERR "error: can't find myfs\n");
        goto failed_mount;
    }

    sb->s_maxbytes = MYFS_MAX_SIZE;
    sb->s_op = &myfs_sops;

    inode = new_inode(sb);
    inode_init_owner(inode, NULL, S_IFDIR | 0744);
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
failed_mount:
    brelse(bh);
failed_sbi:
    sb->s_fs_info = NULL;
    kfree(sbi);
    return ret;
}

static struct dentry* myfs_mount(struct file_system_type *fs_type,
                                 int flags, const char *dev_name,
                                 void *data) {
    return mount_bdev(fs_type, flags, dev_name, data, myfs_fill_super);
}

static struct file_system_type myfs_type = {
    .name = "myfs",
    .owner = THIS_MODULE,
    .mount = myfs_mount,
    .kill_sb = kill_block_super
};

static int __init init_myfs(void) {
    int ret;

    printk(KERN_INFO "Init myfs\n");
    ret = register_filesystem(&myfs_type);
    if (ret != 0) {
        printk(KERN_ERR "Failed to register myfs, err: %d\n", ret);
    }
    return 0;
}

static void __exit exit_myfs(void) {
    int ret;
    printk(KERN_INFO "Exit myfs\n");
    ret = unregister_filesystem(&myfs_type);
    if (ret != 0) {
        printk(KERN_ERR "Failed to unregister myfs, err: %d\n", ret);
    }
}

module_init(init_myfs);
module_exit(exit_myfs);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Feng Junxuan");
MODULE_VERSION("0.1");
