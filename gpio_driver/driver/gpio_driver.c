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

#define DRIVER_NAME "gpio_driver"

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

struct _gpio_pin {
	int pin;
	char *name;
	bool used;
};

/* LED -> IN11(IO-0/PI19) -> 275 */
struct _gpio_pin pin_write = {
	.pin = 275,
	.name = "gpio_write",
	.used = false
};

/* SWT -> PIN13(IO-2/PI18) -> 274 */
struct _gpio_pin pin_read = {
	.pin = 274,
	.name = "gpio_read",
	.used = false
};

static ssize_t
gpio_driver_read(struct file *instance,
		char __user *user, size_t count, loff_t *offset)
{
	dev_info(drv_dev, "gpio_driver_read finished open\n");

	return 0;
}

static ssize_t
gpio_driver_write(struct file *instance,
		const char __user *user, size_t count, loff_t *offset)
{
	dev_info(drv_dev, "gpio_driver_write finished open\n");

	return 0;
}

static int
gpio_driver_open(struct inode *dev_node, struct file *instance)
{
	int err;

	if (instance->f_flags & O_WRONLY) {
		if (pin_write.used) {
			dev_info(drv_dev, "pin already in use\n");
			return -1;
		}

		err = gpio_request(pin_write.pin, pin_write.name);
		if (err) {
			pr_err("gpio_request failed %d\n", err);
			return -1;
		}
		err = gpio_direction_output(pin_write.pin, 0);
		if (err) {
			pr_err("gpio_direction_output failed %d\n", err);
			gpio_free(pin_write.pin);
			return -1;
		}

		pin_write.used = true;
		dev_info(drv_dev, "gpio_driver_open O_WRONLY\n");
	}

	if (instance->f_flags & O_RDONLY) {
		if (pin_read.used) {
			dev_info(drv_dev, "pin already in use\n");
			return -1;
		}

		err = gpio_request(pin_read.pin, pin_read.name);
		if (err) {
			pr_err("gpio_request failed %d\n", err);
			return -1;
		}
		err = gpio_direction_input(pin_read.pin);
		if (err) {
			pr_err("gpio_direction_input failed %d\n", err);
			gpio_free(pin_read.pin);
			return -1;
		}

		pin_read.used = true;
		dev_info(drv_dev, "gpio_driver_open O_RDONLY\n");
	}

	return 0;
}

static int
gpio_driver_close(struct inode *dev_node, struct file *instance)
{
	dev_info(drv_dev, "gpio_driver_closed finished cleanup\n");

	return 0;
}


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = gpio_driver_read,
	.write = gpio_driver_write,
	.open = gpio_driver_open,
	.release = gpio_driver_close,
};

static int __init
gpio_driver_init(void)
{
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

	if (pin_write.used)
		gpio_free(pin_write.pin);

	if (pin_read.used)
		gpio_free(pin_read.pin);

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
