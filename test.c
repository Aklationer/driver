#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 1024
#define procfs_name "helloworld"

static struct proc_dir_entry *out_proc_file;

static char procfs_buffer[PROCFS_MAX_SIZE];

static unsigned long procfs_buffer_size = 0;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    char s[13] = "HelloWorld!\n";
    int len = sizeof(s);
    ssize_t ret = len;

    if (*offset >= len || copy_to_user(buffer, s, len))
    {
        pr_info("copy_to_user failed\n");
        ret = 0;
    }
    else
    {
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name); // 當前file名稱
        *offset += len;
    }

    return ret;
}

static ssize_t procfile_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
    procfs_buffer_size = len;
    if (procfs_buffer_size >= PROCFS_MAX_SIZE)
    {
        procfs_buffer_size = PROCFS_MAX_SIZE - 1;
    }

    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size))
        return -EFAULT;

    procfs_buffer[procfs_buffer_size] = '\0';

    *off += procfs_buffer_size;

    pr_info("procfile write %s\n", procfs_buffer);

    return procfs_buffer_size;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};
#else

static const struct file_operations proc_file_fops = {
    .read = procfile_read,
    .write = procfile_write,
}
#endif

static int __init procfs_init(void)
{
    out_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    if (!out_proc_file)
    {
        pr_err("Error:Could not initialize /proc/%s\n", procfs_name);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", procfs_name);
    return 0;
}

static void __exit procfs_exit(void)
{
    proc_remove(out_proc_file);
    pr_info("/proc/%s removed\n", procfs_name);
}

module_init(procfs_init);
module_exit(procfs_exit);

MODULE_LICENSE("GPL");