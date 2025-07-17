#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AVI FENESH");
MODULE_DESCRIPTION("A simple character device module");
static size_t count = 0;

static struct proc_dir_entry *proc_entry;
static char msg[256] = "Hello from avi_proc_entry!\n";

static ssize_t
proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	size_t msg_len = strlen(msg);
	if (*ppos >= msg_len)
		return 0;
	if (count > msg_len - *ppos)
		count = msg_len - *ppos;
	if (copy_to_user(buf, msg + *ppos, count))
		return -EFAULT;
	*ppos += count;
	return count;
}

static ssize_t
proc_write(struct file *file, const char __user *buf, size_t count,
	   loff_t *ppos)
{
	if (count >= sizeof(msg))
		count = sizeof(msg) - 1;
	if (copy_from_user(msg, buf, count))
		return -EFAULT;
	msg[count] = '\0';
	*ppos += count;
	pr_info("proc_write: %s\n", msg);
	return count;
}

static int
proc_release(struct inode *inode, struct file *file)
{
	count--;
	return 0;
}

static int
proc_open(struct inode *inode, struct file *file)
{
	count++;
	return 0;
}

static const struct proc_ops proc_fops = {
	.proc_read = proc_read,
	.proc_lseek = default_llseek,
	.proc_write = proc_write,
	.proc_open = proc_open,
	.proc_release = proc_release,
};

static int __init
avi_module_init(void)
{
	pr_info("Avi module initializing...\n");
	proc_entry = proc_create("avi_driver", 0666, NULL, &proc_fops);
	if (!proc_entry) {
		pr_err("Failed to create proc entry.\n");
		return -ENOMEM;
	}
	pr_info("Avi module initialized successfully.\n");
	return 0;
}

static void
avi_module_exit(void)
{
	pr_info("Avi module exiting...\n");
	if (proc_entry) {
		if (count > 0) {
			pr_warn("Proc entry is still open, count: %zu\n",
				count);
		}
		proc_remove(proc_entry);
		pr_info("Proc entry removed.\n");
	}
	pr_info("Avi module exited successfully.\n");
}

module_init(avi_module_init);
module_exit(avi_module_exit);
