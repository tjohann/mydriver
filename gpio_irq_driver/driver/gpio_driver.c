/*
 * gpio_irq_driver.c -> simple template driver
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

#define DRIVER_NAME "gpio_irq_driver"

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

static int gpio_irq;
static wait_queue_head_t sleep_wq;
static int irq_event;


static irqreturn_t
gpio_irq_driver_isr(int irq, void *data)
{
	pr_info("gpio_irq_driver_isr irq %d with data %p)\n", irq, data );

	irq_event += 1;
	wake_up(&sleep_wq);

	return IRQ_HANDLED;
}


static irqreturn_t
hard_irq_driver_isr(int irq, void *dev_id)
{
    return IRQ_WAKE_THREAD;
}


static ssize_t
gpio_irq_driver_read(struct file *instance,
		 char __user *user, size_t count, loff_t *offset)
{
	dev_info(drv_dev, "gpio_irq_driver_open finished open\n");

	return 0;
}


static int
gpio_irq_driver_open(struct inode *dev_node, struct file *instance)
{
	dev_info(drv_dev, "gpio_irq_driver_open finished open\n");

	return 0;
}

static int
gpio_irq_driver_close(struct inode *dev_node, struct file *instance)
{
	dev_info(drv_dev, "gpio_irq_driver_closed finished cleanup\n");

	return 0;
}


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = gpio_irq_driver_read,
	.open = gpio_irq_driver_open,
	.release = gpio_irq_driver_close,
};

static int
conf_gpio_pin(int gpio_pin)
{


	pr_info("conf_gpio_pin finished\n");

	return 0;
}

static int __init
gpio_irq_driver_init(void)
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

	/* TODO: not fixed */
	gpio_irq = conf_gpio_pin(0);
	if (gpio_irq == -1)
		goto free_device;

	return 0;

free_device:
	device_destroy(dev_class, dev_number);

free_class:
	class_destroy(dev_class);

free_cdev:
	kobject_put(&dev_object->kobj);

free_dev_number:
	unregister_chrdev_region(dev_number, 1);

	return -EIO;
}

static void __exit
gpio_irq_driver_exit(void)
{
	dev_info(drv_dev, "gpio_irq_driver_exit called\n");

	device_destroy(dev_class, dev_number);
	class_destroy(dev_class);

	cdev_del(dev_object);
	unregister_chrdev_region(dev_number, 1);

	return;
}

module_init(gpio_irq_driver_init);
module_exit(gpio_irq_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gpio_irq_driver - simple template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
