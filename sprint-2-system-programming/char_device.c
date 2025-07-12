#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AVI FENESH");
MODULE_DESCRIPTION("A simple character device module");

static struct proc_dir_entry *custom_proc_entry;
struct proc_ops driver_proc_ops = {};

static int avi_module_init(void)
{
	printk("Avi module initializing...\n");
	custom_proc_entry =
	    proc_create("avi_proc_entry", 0666, NULL, &driver_proc_ops);
	printk("Avi module initialized successfully.\n");
	return 0;
}

static void avi_module_exit(void)
{
	printk("Avi module exiting...\n");
	if (custom_proc_entry) {
		proc_remove(custom_proc_entry);
		printk("Custom proc entry removed.\n");
	}
	printk("Avi module exited successfully.\n");
}

module_init(avi_module_init);
module_exit(avi_module_exit);