#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "lenovo-version"

static const char *version_info = "Version: 1.0.0\n";

static ssize_t version_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    ssize_t ret = 0;
    size_t version_info_size = strlen(version_info);

    if (*offset == 0) {
        if (copy_to_user(buffer, version_info, version_info_size)) {
            return -EFAULT;
        }
        ret = version_info_size;
        *offset += version_info_size;
    }

    return ret;
}

static const struct proc_ops version_fops = {
    .proc_read = version_read,
};

static int __init version_proc_init(void)
{
    //struct proc_dir_entry *proc_parent = proc_mkdir("lenovo", NULL);

    if (!proc_create(PROC_NAME, 0444, NULL, &version_fops)) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit version_proc_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
}

module_init(version_proc_init);
module_exit(version_proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("This module creates a character driver that shows a fixed version string.");
