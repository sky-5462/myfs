#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init init_myfs(void) {
    printk(KERN_INFO "Init myfs\n");
    return 0;
}

static void __exit exit_myfs(void) {
    printk(KERN_INFO "Exit myfs\n");
}

module_init(init_myfs);
module_exit(exit_myfs);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Feng Junxuan");
MODULE_VERSION("0.1");
