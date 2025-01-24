/*
此程式用於在/proc底下新增file，並完成其read和write的操作定義
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/minmax.h>
#include <linux/seq_file.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 1024U
#define procfs_name "helloworld"

static struct proc_dir_entry *out_proc_file;

static char procfs_buffer[PROCFS_MAX_SIZE];

static unsigned long procfs_buffer_size = 0;

static void *seq_start(struct seq_file *s, loff_t *pos)
{
    if(*pos >= procfs_buffer_size)
        return NULL;
    return &procfs_buffer[*pos];
}

static void *seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if(*pos >= procfs_buffer_size)
        return NULL;
    return &procfs_buffer[*pos];
}

static int seq_show(struct seq_file *s, void *v)
{
    char* t = v;
    seq_printf(s, "%c", *t);
    return 0;
}

static void seq_stop(struct seq_file *s, void *v) 
{
   
}

static ssize_t procfile_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
    procfs_buffer_size = min(len,(size_t)PROCFS_MAX_SIZE);
    
    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size))
        return -EFAULT;


    pr_info("procfile write %s\n", procfs_buffer);

    return procfs_buffer_size;
}

static const struct seq_operations my_seq_ops = {
    .start = seq_start,
    .next  = seq_next,
    .stop  = seq_stop,
    .show  = seq_show,
};


static int procfile_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &my_seq_ops);
}


#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = seq_read,
    .proc_write = procfile_write,
    .proc_open = procfile_open,
    .proc_release = seq_release
};
#else

static const struct file_operations proc_file_fops = {
    .read = seq_read,,
    .write = procfile_write,
    .open = procfile_open,
    .release = seq_release,
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

    proc_set_user(out_proc_file, GLOBAL_ROOT_UID, GLOBAL_ROOT_GID); // 把file設為只有root和root group控制

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