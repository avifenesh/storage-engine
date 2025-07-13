#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AVI FENESH");
MODULE_DESCRIPTION("A simple character device module");

static struct proc_dir_entry *proc_entry;

static ssize_t proc_read(struct file *file, char __user *buf, size_t count,
			 loff_t *ppos)
{
	static const char *msg = "Hello from avi_proc_entry!\n";
	size_t len = strlen(msg);
	if (*ppos >= len)
		return 0;
	if (count > len - *ppos)
		count = len - *ppos;
	if (copy_to_user(buf, msg + *ppos, count))
		return -EFAULT;
	*ppos += count;
	return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_lseek = default_llseek,
};

static int __init avi_module_init(void)
{
	pr_info("Avi module initializing...\n");
	proc_entry = proc_create("avi_driver", 0444, NULL, &proc_fops);
	if (!proc_entry) {
		pr_err("Failed to create proc entry.\n");
		return -ENOMEM;
	}
	pr_info("Avi module initialized successfully.\n");
	return 0;
}

static void avi_module_exit(void)
{
	pr_info("Avi module exiting...\n");
	if (proc_entry) {
		proc_remove(proc_entry);
		pr_info("Proc entry removed.\n");
	}
	pr_info("Avi module exited successfully.\n");
}

module_init(avi_module_init);
module_exit(avi_module_exit);