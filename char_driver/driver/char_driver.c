/*
 * char_driver.c -> simple template driver
 *
 * GPL
 * (c) 2013-2016, thorsten.johannvorderbrueggen@t-online.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

/* common defines for driver and usage */
#include "../common.h"

#define DRIVER_NAME "char_driver"

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

char data_s[] = "char_driver says hello crude world!";

struct _instance_data {
	int count;
	char *data_s;
};
#define SD struct _instance_data


static ssize_t
char_driver_read(struct file *instance,
		 char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
	unsigned long to_copy;
	unsigned long copied;

	SD *data_p = (SD*) instance->private_data;

	to_copy = min(count, (size_t) data_p->count);
	not_copied = copy_to_user(user, data_p->data_s, to_copy);

	copied = to_copy - not_copied;
	data_p->count -= copied;
	*offset += copied;

	dev_info(drv_dev, "copied %d bytes in *_driver_read to user \"%s\"\n",
		 (int) copied, data_p->data_s);

	return copied;
}

static ssize_t
char_driver_write(struct file *instance,
		  const char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
	unsigned long to_copy;
	unsigned long copied;

	SD *data_p = (SD*) instance->private_data;

	char data[256];
	memset(data, 0, sizeof(data));

	to_copy = min(count, sizeof(data));
	not_copied = copy_from_user(data, user, to_copy);

	copied = to_copy - not_copied;
	*offset += copied;

	dev_info(drv_dev, "copied %d bytes in *_driver_write from user \"%s\"\n",
		(int) copied, data);

	if(data_p->data_s != NULL)
		kfree(data_p->data_s);

	data_p->data_s = (char *) kmalloc(copied + 1, GFP_USER);
	if (data_p->data_s == NULL) {
		pr_err("kmalloc in *_driver_write\n");
		return  -ENOMEM;
	}

	memcpy(data_p->data_s, data, copied);
	data_p->data_s[copied] = '\0';
	data_p->count = copied;

	return copied;
}

static int
char_driver_open(struct inode *dev_node, struct file *instance)
{
	size_t len = strlen(data_s) + 1;
	SD *data_p = (SD *) kmalloc(sizeof(SD), GFP_USER);
	if (data_p == NULL) {
		pr_err("kmalloc in *_driver_open\n");
		return -ENOMEM;
	}

	data_p->data_s = (char *) kmalloc(len, GFP_USER);
	if (data_p->data_s == NULL) {
		pr_err("kmalloc in *_driver_open\n");
		return -ENOMEM;
	}

	memcpy(data_p->data_s, data_s, len);
	data_p->data_s[len] = '\0';

	data_p->count = len;
	instance->private_data = (void *) data_p;

	dev_info(drv_dev, "char_driver_open finished open\n");

	return 0;
}

static int
char_driver_close(struct inode *dev_node, struct file *instance)
{
	SD *data_p = NULL;

	if (instance->private_data) {
		data_p = (SD *) instance->private_data;

		if (data_p->data_s != NULL)
			kfree(data_p->data_s);

		kfree(instance->private_data);
	}

	dev_info(drv_dev, "char_driver_closed finished cleanup\n");

	return 0;
}

static long
char_driver_ioctl(struct file *instance, unsigned int cmd, unsigned long __user arg)
{
	unsigned long not_copied;
	unsigned long to_copy;
	unsigned long copied;

	SD *data_p = (SD *) instance->private_data;
	char *data;
	char *tmp_data;

	switch(cmd) {
	case IOCTL_SET_DATA:
		data = (char *) arg;
		to_copy = strlen(data);

		tmp_data = (char *) kmalloc(to_copy + 1, GFP_USER);
		if (data_p->data_s == NULL) {
			pr_err("kmalloc in *_driver_ioctl\n");
			return  -ENOMEM;
		}

		not_copied = copy_from_user(tmp_data, data, to_copy);
		copied = to_copy - not_copied;

		if(data_p->data_s != NULL)
			kfree(data_p->data_s);
		data_p->data_s = tmp_data;

		data_p->data_s[copied] = '\0';
		data_p->count = copied;

		break;
	default:
		pr_err("unknown ioctl 0x%x\n", cmd);
		return -EINVAL;
	}

	dev_info(drv_dev, "char_driver_ioctl finished\n");

	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = char_driver_read,
	.write = char_driver_write,
	.open = char_driver_open,
	.unlocked_ioctl = char_driver_ioctl,
	.release = char_driver_close,
};


#ifdef CONFIG_PM
static int
char_driver_suspend(struct device *dev, pm_message_t state)
{
	switch (state.event) {
	case PM_EVENT_ON:
		dev_dbg(dev, "on event\n");
		break;
	case PM_EVENT_FREEZE:
		dev_dbg(dev, "freeze event\n");
		break;
	case PM_EVENT_SUSPEND:
		dev_dbg(dev, "suspend event\n");
	        break;
	case PM_EVENT_HIBERNATE:
		dev_dbg(dev,"hibernate...\n");
		break;
	default:
		dev_dbg(dev,"no valid pm event: 0x%x\n", state.event);
		break;
	}

	dev_info(dev,"char_driver_suspend(%p) finished \n", dev );

	return 0;
}

static int
char_driver_resume(struct device *dev)
{
	dev_info(dev, "char_driver_resume(%p) finished \n",dev);

	return 0;
}
#endif

static int __init
char_driver_init(void)
{
/*
 * - first get a device number
 * - alloc a driver object
 * - init driver object
 * - add driver object to kernel
 *
 * - create sysfs entry (/sys/devices/virtual/char_driver/char_driver)
 * - create /dev entry via udev (/etc/udev/rules/90-<modulename>.rules)
 *   entry -> KERNEL=="char_driver", MODE="0666"
 * -> see mydriver.git/udev
 */
	pr_info("char_driver_init called\n");

	/* get a device number */
	if (alloc_chrdev_region(&dev_number, 0, 1, DRIVER_NAME) < 0)
		return -EIO;

	dev_object = cdev_alloc();
	if (dev_object == NULL)
		goto free_dev_number;

	dev_object->owner = THIS_MODULE;
	dev_object->ops = &fops;

	if (cdev_add(dev_object, dev_number, 1)) {
		pr_err("an error occured -> cdev_add()\n");
		goto free_cdev;
	}

	/* add sysfs/udev entry */
	dev_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(dev_class)) {
		pr_err("an error occured -> class_create()\n");
		goto free_cdev;
	}
#ifdef CONFIG_PM
	dev_class->suspend = char_driver_suspend;
	dev_class->resume = char_driver_resume;
#endif
	drv_dev = device_create(dev_class, NULL, dev_number, NULL, "%s",
				DRIVER_NAME);
	if (IS_ERR(drv_dev)) {
		pr_err("an error occured -> device_create()\n");
		goto free_class;
	}

	return 0;

free_class:
	class_destroy(dev_class);

free_cdev:
	kobject_put(&dev_object->kobj);

free_dev_number:
	unregister_chrdev_region(dev_number, 1);

	return -EIO;
}

static void __exit
char_driver_exit(void)
{
	dev_info(drv_dev, "char_driver_exit called\n");

	device_destroy(dev_class, dev_number);
	class_destroy(dev_class);

	cdev_del(dev_object);
	unregister_chrdev_region(dev_number, 1);

	return;
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("char_driver - simple template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
