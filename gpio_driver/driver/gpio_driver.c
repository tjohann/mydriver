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

#include "../common.h"

#define DRIVER_NAME "gpio_driver"

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

struct _instance_data {
	int pin;
	char *name;
	bool used;
};
#define SD struct _instance_data

static int
config_pin(int pin, bool write_pin)
{
/*	snprintf(name, sizeof(name), "gpio-write-%d", gpionr );
		
	err = gpio_request(DEF_PIN_WRITE, name.name);
	if (err) {
		pr_err("gpio_request failed %d\n", err);
		kfree(name);
		return -1;
	}
	err = gpio_direction_output(DEF_PIN_WRITE, 0);
	if (err) {
		pr_err("gpio_direction_output failed %d\n", err);
		gpio_free(DEF_PIN_WRITE);
		kfree(name);
		return -1;
	}
	
	SD *data_p = (SD *) kmalloc(sizeof(SD), GFP_USER);
	if (data_p == NULL) {
		pr_err("kmalloc in *_driver_open\n");
		return -ENOMEM;
	}
	
	pin_write.used = true;
	


	snprintf(name, sizeof(name), "gpio-write-%d", gpionr );
		
		err = gpio_request(DEF_PIN_WRITE, name.name);
		if (err) {
			pr_err("gpio_request failed %d\n", err);
			kfree(name);
			return -1;
		}
		err = gpio_direction_output(DEF_PIN_WRITE, 0);
		if (err) {
			pr_err("gpio_direction_output failed %d\n", err);
			gpio_free(DEF_PIN_WRITE);
			kfree(name);
			return -1;
		}

		SD *data_p = (SD *) kmalloc(sizeof(SD), GFP_USER);
		if (data_p == NULL) {
			pr_err("kmalloc in *_driver_open\n");
			return -ENOMEM;
		}
		
		pin_write.used = true;
*/
	
	return 0;
}

static ssize_t
gpio_driver_read(struct file *instance,
		 char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
        unsigned long to_copy;

	u32 value = 0;

	value = gpio_get_value(pin_read.pin);

        to_copy = min(count, sizeof(value));
        not_copied = copy_to_user(user, &value, to_copy);

        return to_copy - not_copied;
}

static ssize_t
gpio_driver_write(struct file *instance,
		  const char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
        unsigned long to_copy;

	u32 value = 0;

        to_copy = min(count, sizeof(value));
        not_copied = copy_from_user(&value, user, to_copy);

	gpio_set_value(pin_write.pin, value ? 1 : 0);

        return to_copy - not_copied;
}

static int
gpio_driver_open(struct inode *dev_node, struct file *instance)
{
	int err;
	char name[15];
	
	int accmode = (instance->f_flags & O_ACCMODE);
	bool read_mode  = (accmode == O_RDONLY);
	bool write_mode = (accmode == O_WRONLY);

	memset(name, 0, sizeof(name));
	
	if (!read_mode && !write_mode) {
		dev_err(drv_dev, "only O_RDONLY and O_WRONLY allowed\n");
		return -EIO;
	}

	if (write_mode) {
		if (config_pin(DEF_PIN_WRITE, true) == -1)
			return -EIO;
		
		dev_info(drv_dev, "gpio_driver_open O_WRONLY\n");
	}

	if (read_mode) {
		if (config_pin(DEF_PIN_READ, false) == -1)
			return -EIO;
		
		dev_info(drv_dev, "gpio_driver_open O_RDONLY\n");
	}

	return 0;
}

static int
gpio_driver_close(struct inode *dev_node, struct file *instance)
{
	dev_info(drv_dev, "gpio_driver_closed finished cleanup\n");

	if (pin_write.used) {
		gpio_free(pin_write.pin);
		pin_write.used = false;
	}

	if (pin_read.used) {
		gpio_free(pin_read.pin);
		pin_read.used = false;
	}

	return 0;
}

static long
gpio_driver_ioctl(struct file *instance, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
	case IOCTL_SET_WRITE_PIN:
		pr_info("IOCTL_SET_WRITE_PIN ... \n");
		break;
	case IOCTL_SET_READ_PIN:
		pr_info("IOCTL_SET_READ_PIN ... \n");
		break;
	default:
		pr_err("unknown ioctl 0x%x\n", cmd);
		return -EINVAL;
	}

	dev_info(drv_dev, "gpio_driver_ioctl finished\n");
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
