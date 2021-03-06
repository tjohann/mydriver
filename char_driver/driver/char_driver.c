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
#include <linux/uaccess.h>
#include <linux/slab.h>

/* common defines for driver and usage */
#include "../common.h"

static const char char_driver_name[] = "char_driver";
static const char class_name[] = "char_driver_class";

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

char data_s[] = "char_driver says hello crude world!";

struct _instance_data {
	int count;
	unsigned long offset;
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
	data_p->offset += copied;
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

	dev_info(drv_dev, "write: copied %d bytes in *_driver_write from user \"%s\"\n",
		 (int) copied, data);

	if(data_p->data_s != NULL)
		kfree(data_p->data_s);

	data_p->data_s = (char *) kmalloc(copied + 1, GFP_USER);
	if (data_p->data_s == NULL) {
		dev_err(drv_dev, "write: kmalloc\n");
		return  -ENOMEM;
	}

	memcpy(data_p->data_s, data, copied);
	data_p->data_s[copied] = '\0';
	data_p->count = copied;
	data_p->offset += copied;

	return copied;
}

static int
char_driver_open(struct inode *dev_node, struct file *instance)
{
	size_t len = strlen(data_s) + 1;
	SD *data_p = (SD *) kmalloc(sizeof(SD), GFP_USER);
	if (data_p == NULL) {
		dev_err(drv_dev, "open: kmalloc\n");
		return -ENOMEM;
	}

	memset(data_p, 0, sizeof(SD));

	data_p->data_s = (char *) kmalloc(len, GFP_USER);
	if (data_p->data_s == NULL) {
		dev_err(drv_dev, "open: kmalloc\n");
		return -ENOMEM;
	}

	memcpy(data_p->data_s, data_s, len);
	data_p->data_s[len] = '\0';

	data_p->count = len;
	instance->private_data = (void *) data_p;

	dev_info(drv_dev, "open finished\n");

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

	dev_info(drv_dev, "close finished\n");

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
			dev_err(drv_dev, "ioctl: kmalloc\n");
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
		dev_err(drv_dev, "unknown ioctl 0x%x\n", cmd);
		return -EINVAL;
	}

	dev_info(drv_dev, "ioctl finished\n");

	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = char_driver_read,
	.write = char_driver_write,
	.open = char_driver_open,
	.unlocked_ioctl = char_driver_ioctl,
	.release = char_driver_close
};


#ifdef CONFIG_PM
static int
char_driver_suspend(struct device *dev, pm_message_t state)
{
	switch (state.event) {
	case PM_EVENT_ON:
		dev_dbg(drv_dev, "on event\n");
		break;
	case PM_EVENT_FREEZE:
		dev_dbg(drv_dev, "freeze event\n");
		break;
	case PM_EVENT_SUSPEND:
		dev_dbg(drv_dev, "suspend event\n");
	        break;
	case PM_EVENT_HIBERNATE:
		dev_dbg(drv_dev, "hibernate...\n");
		break;
	default:
		dev_dbg(drv_dev,"no valid pm event: 0x%x\n", state.event);
		break;
	}

	dev_info(drv_dev,"suspend finished\n");

	return 0;
}

static int
char_driver_resume(struct device *dev)
{
	dev_info(drv_dev, "resume(%p) finished\n", dev);

	return 0;
}
#endif

static int __init
char_driver_init(void)
{
/*
 * - create sysfs entry (/sys/devices/virtual/char_driver/char_driver)
 * - create /dev entry via udev (/etc/udev/rules/90-<modulename>.rules)
 *   entry -> KERNEL=="char_driver", MODE="0666"
 */
	if (alloc_chrdev_region(&dev_number, 0, 1, char_driver_name) < 0)
		return -EIO;

	dev_object = cdev_alloc();
	if (dev_object == NULL)
		goto free_dev_number;

	dev_object->owner = THIS_MODULE;
	dev_object->ops = &fops;

	if (cdev_add(dev_object, dev_number, 1)) {
		dev_err(drv_dev, "cdev_add\n");
		goto free_cdev;
	}

	/* add sysfs/udev entry */
	dev_class = class_create(THIS_MODULE, class_name);
	if (IS_ERR(dev_class)) {
		dev_err(drv_dev, "class_create\n");
		goto free_cdev;
	}
#ifdef CONFIG_PM
	dev_class->suspend = char_driver_suspend;
	dev_class->resume = char_driver_resume;
#endif
	drv_dev = device_create(dev_class, NULL, dev_number, NULL, "%s",
				char_driver_name);
	if (IS_ERR(drv_dev)) {
		dev_err(drv_dev, "device_create\n");
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
	device_destroy(dev_class, dev_number);
	class_destroy(dev_class);

	cdev_del(dev_object);
	unregister_chrdev_region(dev_number, 1);

	return;
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_VERSION("1.0");
MODULE_ALIAS("my_char_driver");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("char_driver - simple char template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
