#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "myfs.h"

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
    inode->i_mode = S_IFDIR | 0744;
    inode->i_size = 0;
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
