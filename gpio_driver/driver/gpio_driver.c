/*
 * gpio_driver.c -> simple template driver
 *
 * GPL
 * (c) 2016, thorsten.johannvorderbrueggen@t-online.de
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
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "../common.h"

#define DRIVER_NAME "gpio_driver"

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

struct _instance_data {
	int pin;
	char *name;
};
#define SD struct _instance_data

static int
config_pin(int pin, bool write_pin, SD **data)
{
	int err = -1;
	size_t len = 0;
	char *name = NULL;

	char tmp_name[15];
	memset(tmp_name, 0, sizeof(tmp_name));

	if (write_pin) {
		snprintf(tmp_name, sizeof(tmp_name), "gpio-write-%d", pin);
		len = strlen(tmp_name) + 1;
	} else {
		snprintf(tmp_name, sizeof(tmp_name), "gpio-read-%d", pin);
		len = strlen(tmp_name) + 1;
	}

	name = (char *) kmalloc(len, GFP_USER);
	if (name == NULL) {
		pr_err("kmalloc in config_pin\n");
		return -1;
	}

	memcpy(name, tmp_name, len);
	name[len] = '\0';

	err = gpio_request(pin, name);
	if (err) {
		pr_err("gpio_request failed %d\n", err);
		goto free_name;
	}

	if (write_pin)
		err = gpio_direction_output(pin, 0);
	else
		err = gpio_direction_input(pin);

	if (err) {
		pr_err("gpio_direction failed %d\n", err);
		goto free_pin;
	}

	*data = (SD *) kmalloc(sizeof(SD), GFP_USER);
	if (*data == NULL) {
		pr_err("kmalloc in config_pin\n");
		goto free_pin;
	}

	(*data)->name = name;
	(*data)->pin = pin;

	return 0;

free_pin:
	gpio_free(pin);

free_name:
	kfree(name);

	return -1;
}

static ssize_t
gpio_driver_read(struct file *instance,
		 char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
        unsigned long to_copy;
	u32 value = 0;

	if (instance->private_data) {
		SD *data = (SD*) instance->private_data;

		value = gpio_get_value(data->pin);

		to_copy = min(count, sizeof(value));
		not_copied = copy_to_user(user, &value, to_copy);

		return to_copy - not_copied;
	} else {
		pr_err("instance->private_data == NULL");
		return -1;
	}
}

static ssize_t
gpio_driver_write(struct file *instance,
		  const char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
        unsigned long to_copy;
	u32 value = 0;

	if (instance->private_data) {
		SD *data = (SD*) instance->private_data;

		to_copy = min(count, sizeof(value));
		not_copied = copy_from_user(&value, user, to_copy);

		gpio_set_value(data->pin, value ? 1 : 0);

		return to_copy - not_copied;
	} else {
		pr_err("instance->private_data == NULL");
		return -1;
	}
}

static int
gpio_driver_open(struct inode *dev_node, struct file *instance)
{
	SD *data = NULL;

	int accmode = (instance->f_flags & O_ACCMODE);
	bool read_mode  = (accmode == O_RDONLY);
	bool write_mode = (accmode == O_WRONLY);

	if (!read_mode && !write_mode) {
		dev_err(drv_dev, "only O_RDONLY and O_WRONLY allowed\n");
		return -EIO;
	}

	if (write_mode) {
		if (config_pin(DEF_PIN_WRITE, true, &data) == -1)
			return -EIO;

		dev_info(drv_dev, "gpio_driver_open O_WRONLY\n");
	}

	if (read_mode) {
		if (config_pin(DEF_PIN_READ, false, &data) == -1)
			return -EIO;

		dev_info(drv_dev, "gpio_driver_open O_RDONLY\n");
	}

	instance->private_data = (void *) data;

	/* some useful info */
	pr_info("gpio_driver_open values:\n");
	pr_info("name = %s\n", data->name);
	pr_info("pin = %d\n", data->pin);

	return 0;
}

static int
gpio_driver_close(struct inode *dev_node, struct file *instance)
{
	SD *data = NULL;

	if (instance->private_data) {
		data = (SD *) instance->private_data;

		/* clear pin */
		gpio_set_value(data->pin, 0);
		
		gpio_free(data->pin);

		kfree(data->name);
		kfree(instance->private_data);
	} else {
		pr_err("instance->private_data == NULL\n");
	}

	dev_info(drv_dev, "gpio_driver_closed finished cleanup\n");

	return 0;
}

static long
gpio_driver_ioctl(struct file *instance, unsigned int cmd, unsigned long  __user arg)
{
	unsigned int value = 0;

	SD *data = NULL;
	SD *tmp_data = NULL;

	if (get_user(value, (int __user *) arg)) {
		pr_err("could not copy from userspace");
		return -EFAULT;
	}

	if (value <= 0) {
		pr_err("a value below <=0 makes no sense\n");
		return -EINVAL;
	} else {
		pr_info("value from userspace is %d", value);
	}

	switch(cmd) {
	case IOCTL_SET_WRITE_PIN:
		if (config_pin(value, true, &data) == -1)
			return -EIO;
		break;
	case IOCTL_SET_READ_PIN:
		if (config_pin(value, false, &data) == -1)
			return -EIO;
		break;
	default:
		pr_err("unknown ioctl 0x%x\n", cmd);
		return -EINVAL;
	}

	if (instance->private_data) {
		tmp_data = (SD *) instance->private_data;

		gpio_free(tmp_data->pin);

		kfree(tmp_data->name);
		kfree(instance->private_data);

	} else {
		pr_err("instance->private_data == NULL\n");
	}

	instance->private_data = (void *) data;

	/* some useful info  */
	pr_info("gpio_driver_ioctl values:\n");
	pr_info("name = %s\n", data->name);
	pr_info("pin = %d\n", data->pin);

	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = gpio_driver_read,
	.write = gpio_driver_write,
	.open = gpio_driver_open,
	.unlocked_ioctl = gpio_driver_ioctl,
	.release = gpio_driver_close,
};

static int __init
gpio_driver_init(void)
{
	pr_info("gpio_driver_init called\n");

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
gpio_driver_exit(void)
{
	dev_info(drv_dev, "gpio_driver_exit called\n");

	device_destroy(dev_class, dev_number);
	class_destroy(dev_class);

	cdev_del(dev_object);
	unregister_chrdev_region(dev_number, 1);

	return;
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gpio_driver - simple template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
