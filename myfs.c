#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int myfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct inode* inode;
    inode = new_inode(sb);
    inode->i_mode = S_IFDIR | 0744;
    inode->i_size = 0;
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
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
