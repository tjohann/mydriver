/*
 * gpio_irq_driver_simple.c -> simple template driver
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

#define DRIVER_NAME "gpio_irq_driver_simple"
/* Pin16/IO-4/PH20 */
#define DEF_PIN_READ  244

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;
static struct device *drv_dev;

int gpio_irq;
int irq_event;
wait_queue_head_t sleep_wq;


static irqreturn_t
hard_irq_driver_isr(int irq, void *data)
{
	irq_event += 1;
	wake_up(&sleep_wq);
	
	return IRQ_HANDLED;
}

static int
config_pin(int pin)
{
	int err = -1, gpio_irq = 0;

	char name[15];
	memset(name, 0, sizeof(name));

	snprintf(name, sizeof(name), "gpio-read-%d", pin);
	
	err = gpio_request(pin, name);
	if (err) {
		pr_err("gpio_request failed %d\n", err);
		return -1;
	}

	err = gpio_direction_input(pin);
	if (err) {
		pr_err("gpio_direction failed %d\n", err);
		goto free_pin;
	}

	gpio_irq = gpio_to_irq(pin);
	if (gpio_irq < 0)
		goto free_pin;
	
	err = request_irq(gpio_irq, hard_irq_driver_isr,
			  IRQF_TRIGGER_FALLING,
			  DRIVER_NAME, dev_object);
	if (err < 0) {
		dev_err(drv_dev, "irq %d busy? error %d\n", pin, err);
		goto free_pin;
	}

	return gpio_irq;

free_pin:
	gpio_free(pin);

	return -1;
}

static ssize_t
gpio_irq_driver_read(struct file *instance,
		 char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
        unsigned long to_copy;
	
	irq_event = 0;
	wait_event_interruptible(sleep_wq, irq_event);	
	to_copy = min(count, sizeof(irq_event));
	not_copied = copy_to_user(user, &irq_event, to_copy);
	
	return to_copy - not_copied;
}

static int
gpio_irq_driver_open(struct inode *dev_node, struct file *instance)
{
	pr_info("gpio_irq_driver_open called\n");

	return 0;
}

static int
gpio_irq_driver_close(struct inode *dev_node, struct file *instance)
{
	pr_info("gpio_irq_driver_closed called\n");

	return 0;
}


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = gpio_irq_driver_read,
	.open = gpio_irq_driver_open,
	.release = gpio_irq_driver_close,
};

static int __init
gpio_irq_driver_init(void)
{
	init_waitqueue_head(&sleep_wq);
	
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

        /* add sysfs entry */
	dev_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(dev_class)) {
		pr_err("an error occured -> class_create()\n");
		goto free_cdev;
	}

	/* add udev entry */
	drv_dev = device_create(dev_class, NULL, dev_number, NULL, "%s",
				DRIVER_NAME);
	if (IS_ERR(drv_dev)) {
		pr_err("an error occured -> device_create()\n");
		goto free_class;
	}

	/* setup gpio for irq */
	gpio_irq = config_pin(DEF_PIN_READ);
	if (gpio_irq == -1) {
		pr_err("an config_pin error occured\n");
		goto free_device;
	}
	
	pr_info("gpio_irq_driver_init with IRQ %d on PIN %d\n",
		gpio_irq, DEF_PIN_READ);
	
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
	
	free_irq(gpio_irq, dev_object);
	gpio_free( DEF_PIN_READ);
	
	return;
}

module_init(gpio_irq_driver_init);
module_exit(gpio_irq_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gpio_irq_driver_simple - simple template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
