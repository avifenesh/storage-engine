#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AVI FENESH");
MODULE_DESCRIPTION("A simple character device module");
static size_t count = 0;


static const struct file_operation = {
	.owner = THIS_MODULE,
	.open = avi_open,
	.release = avi_release,
	.write = avi_write,
	.read = avi_read,
};

static int __init
avi_module_init(void)
{
	pr_info("Avi module initializing...\n");
	result = register_chrdev(0, "avi_char_device", &file_operations);
	return result < 0 ? result : 0;
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
