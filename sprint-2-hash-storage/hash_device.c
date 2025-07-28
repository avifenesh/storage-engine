/*
 * Hash Storage Device Driver - Character Device Interface
 *
 * Provides /dev/storage-hash character device with IOCTL interface
 * for high-performance hash table operations in kernel space.
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "storage-hash"
#define CLASS_NAME "storage_engine"

/* IOCTL command definitions */
#define HASH_IOC_MAGIC 'H'
#define HASH_IOC_PUT _IOW(HASH_IOC_MAGIC, 1, struct hash_operation)
#define HASH_IOC_GET _IOWR(HASH_IOC_MAGIC, 2, struct hash_operation)
#define HASH_IOC_DELETE _IOW(HASH_IOC_MAGIC, 3, struct hash_operation)
#define HASH_IOC_STATS _IOR(HASH_IOC_MAGIC, 4, struct hash_stats)
#define HASH_IOC_MAXNR 4

/* Data structures for IOCTL operations */
struct hash_operation {
	void __user *key;
	size_t key_len;
	void __user *value;
	size_t value_len;
};

struct hash_stats {
	uint32_t item_count;
	uint32_t bucket_count;
	uint32_t memory_usage;
};

/* External function declarations from hash_engine.c */
extern int hash_engine_init(uint32_t bucket_count);
extern void hash_engine_cleanup(void);
extern int hash_put(const void *key, size_t key_len, const void *value,
		    size_t value_len);
extern int hash_get(const void *key, size_t key_len, void **value,
		    size_t *value_len);
extern int hash_delete(const void *key, size_t key_len);
extern int hash_get_stats(uint32_t *item_count, uint32_t *bucket_count,
			  uint32_t *memory_usage);

/* Device driver variables */
static int major_number;
static struct class *hash_class = NULL;
static struct device *hash_device = NULL;
static struct cdev hash_cdev;
static dev_t hash_dev_number;

static int
device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "storage-hash: Device opened\n");
	return 0;
}

static int
device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "storage-hash: Device closed\n");
	return 0;
}

static ssize_t
device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
	/* This device is IOCTL-only, no read operation */
	return -ENOSYS;
}

static ssize_t
device_write(struct file *file, const char __user *buffer, size_t len,
	     loff_t *offset)
{
	/* This device is IOCTL-only, no write operation */
	return -ENOSYS;
}

/*
 * Handle PUT operation - store key-value pair
 */
static long
handle_put_operation(struct hash_operation __user *arg)
{
	struct hash_operation op;
	void *kernel_key = NULL;
	void *kernel_value = NULL;
	int ret;

	if (copy_from_user(&op, arg, sizeof(op)))
		return -EFAULT;

	if (op.key_len == 0 || op.value_len == 0 || op.key_len > 256
	    || op.value_len > 4096)
		return -EINVAL;

	/* Allocate kernel buffers */
	kernel_key = kmalloc(op.key_len, GFP_KERNEL);
	if (!kernel_key)
		return -ENOMEM;

	kernel_value = kmalloc(op.value_len, GFP_KERNEL);
	if (!kernel_value) {
		kfree(kernel_key);
		return -ENOMEM;
	}

	/* Copy data from user space */
	if (copy_from_user(kernel_key, op.key, op.key_len)) {
		ret = -EFAULT;
		goto cleanup;
	}

	if (copy_from_user(kernel_value, op.value, op.value_len)) {
		ret = -EFAULT;
		goto cleanup;
	}

	/* Perform hash operation */
	ret = hash_put(kernel_key, op.key_len, kernel_value, op.value_len);

cleanup:
	kfree(kernel_key);
	kfree(kernel_value);
	return ret;
}

/*
 * Handle GET operation - retrieve value by key
 */
static long
handle_get_operation(struct hash_operation __user *arg)
{
	struct hash_operation op;
	void *kernel_key = NULL;
	void *kernel_value = NULL;
	size_t value_len;
	int ret;

	if (copy_from_user(&op, arg, sizeof(op)))
		return -EFAULT;

	if (op.key_len == 0 || op.key_len > 256)
		return -EINVAL;

	/* Allocate kernel buffer for key */
	kernel_key = kmalloc(op.key_len, GFP_KERNEL);
	if (!kernel_key)
		return -ENOMEM;

	/* Copy key from user space */
	if (copy_from_user(kernel_key, op.key, op.key_len)) {
		ret = -EFAULT;
		goto cleanup;
	}

	/* Perform hash operation */
	ret = hash_get(kernel_key, op.key_len, &kernel_value, &value_len);
	if (ret)
		goto cleanup;

	/* Check if user buffer is large enough */
	if (op.value_len < value_len) {
		ret = -ENOSPC;
		goto cleanup_value;
	}

	/* Copy value back to user space */
	if (copy_to_user(op.value, kernel_value, value_len)) {
		ret = -EFAULT;
		goto cleanup_value;
	}

	/* Update the operation structure with actual value length */
	op.value_len = value_len;
	if (copy_to_user(arg, &op, sizeof(op))) {
		ret = -EFAULT;
		goto cleanup_value;
	}

cleanup_value:
	if (kernel_value)
		kfree(kernel_value);
cleanup:
	kfree(kernel_key);
	return ret;
}

/*
 * Handle DELETE operation - remove key-value pair
 */
static long
handle_delete_operation(struct hash_operation __user *arg)
{
	struct hash_operation op;
	void *kernel_key = NULL;
	int ret;

	if (copy_from_user(&op, arg, sizeof(op)))
		return -EFAULT;

	if (op.key_len == 0 || op.key_len > 256)
		return -EINVAL;

	/* Allocate kernel buffer for key */
	kernel_key = kmalloc(op.key_len, GFP_KERNEL);
	if (!kernel_key)
		return -ENOMEM;

	/* Copy key from user space */
	if (copy_from_user(kernel_key, op.key, op.key_len)) {
		ret = -EFAULT;
		goto cleanup;
	}

	/* Perform hash operation */
	ret = hash_delete(kernel_key, op.key_len);

cleanup:
	kfree(kernel_key);
	return ret;
}

/*
 * Handle STATS operation - get hash table statistics
 */
static long
handle_stats_operation(struct hash_stats __user *arg)
{
	struct hash_stats stats;
	int ret;

	ret = hash_get_stats(&stats.item_count, &stats.bucket_count,
			     &stats.memory_usage);
	if (ret)
		return ret;

	if (copy_to_user(arg, &stats, sizeof(stats)))
		return -EFAULT;

	return 0;
}

/*
 * IOCTL handler
 */
static long
device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	/* Verify IOCTL command */
	if (_IOC_TYPE(cmd) != HASH_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > HASH_IOC_MAXNR)
		return -ENOTTY;

	/* Verify user space pointer access */
	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	if (ret)
		return -EFAULT;

	/* Handle specific IOCTL commands */
	switch (cmd) {
	case HASH_IOC_PUT:
		ret = handle_put_operation((struct hash_operation __user *)arg);
		break;

	case HASH_IOC_GET:
		ret = handle_get_operation((struct hash_operation __user *)arg);
		break;

	case HASH_IOC_DELETE:
		ret = handle_delete_operation(
		    (struct hash_operation __user *)arg);
		break;

	case HASH_IOC_STATS:
		ret = handle_stats_operation((struct hash_stats __user *)arg);
		break;

	default:
		ret = -ENOTTY;
		break;
	}

	return ret;
}

/* File operations structure */
static struct file_operations fops = {
	.open = device_open,
	.read = device_read,
	.write = device_write,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
	.owner = THIS_MODULE,
};

/*
 * Module initialization
 */
static int __init
hash_device_init(void)
{
	int ret;

	printk(KERN_INFO "storage-hash: Initializing device driver\n");

	/* Initialize hash engine */
	ret = hash_engine_init(1024);
	if (ret) {
		printk(KERN_ALERT
		       "storage-hash: Failed to initialize hash engine\n");
		return ret;
	}

	/* Allocate major number dynamically */
	ret = alloc_chrdev_region(&hash_dev_number, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		printk(KERN_ALERT
		       "storage-hash: Failed to allocate major number\n");
		hash_engine_cleanup();
		return ret;
	}
	major_number = MAJOR(hash_dev_number);

	/* Initialize character device */
	cdev_init(&hash_cdev, &fops);
	hash_cdev.owner = THIS_MODULE;

	/* Add character device to system */
	ret = cdev_add(&hash_cdev, hash_dev_number, 1);
	if (ret < 0) {
		printk(KERN_ALERT
		       "storage-hash: Failed to add character device\n");
		unregister_chrdev_region(hash_dev_number, 1);
		hash_engine_cleanup();
		return ret;
	}

	/* Create device class */
	hash_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(hash_class)) {
		printk(KERN_ALERT
		       "storage-hash: Failed to create device class\n");
		cdev_del(&hash_cdev);
		unregister_chrdev_region(hash_dev_number, 1);
		hash_engine_cleanup();
		return PTR_ERR(hash_class);
	}

	/* Create device */
	hash_device = device_create(hash_class, NULL, hash_dev_number, NULL,
				    DEVICE_NAME);
	if (IS_ERR(hash_device)) {
		printk(KERN_ALERT "storage-hash: Failed to create device\n");
		class_destroy(hash_class);
		cdev_del(&hash_cdev);
		unregister_chrdev_region(hash_dev_number, 1);
		hash_engine_cleanup();
		return PTR_ERR(hash_device);
	}

	printk(KERN_INFO
	       "storage-hash: Device driver initialized successfully\n");
	printk(KERN_INFO "storage-hash: Major number: %d\n", major_number);
	printk(KERN_INFO "storage-hash: Device created: /dev/%s\n",
	       DEVICE_NAME);

	return 0;
}

/*
 * Module cleanup
 */
static void __exit
hash_device_exit(void)
{
	printk(KERN_INFO "storage-hash: Cleaning up device driver\n");

	/* Clean up in reverse order */
	device_destroy(hash_class, hash_dev_number);
	class_destroy(hash_class);
	cdev_del(&hash_cdev);
	unregister_chrdev_region(hash_dev_number, 1);
	hash_engine_cleanup();

	printk(KERN_INFO "storage-hash: Device driver cleanup complete\n");
}

module_init(hash_device_init);
module_exit(hash_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM Storage Engine Learning Project");
MODULE_DESCRIPTION("Hash Table Character Device Driver");
MODULE_VERSION("1.0");