/*
 * i2c_gpio_driver_simple.c -> simple template driver
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
#include <linux/i2c.h>
#include <linux/slab.h>

#define DRIVER_NAME "i2c_gpio_driver_simple"

#define WRITE_PORT 0x01
#define READ_PORT  0x02

static dev_t pcf8574_dev_number;
static struct cdev *pcf8574_object;
struct class *pcf8574_class;
static struct device *drv_dev;

struct _instance_data {
	struct i2c_client *slave;
	struct i2c_adapter *adapter;
	int pin_irq;
	int gpio_irq;
	int adapter_nr;
	unsigned short addr;
	unsigned char direction;
};
#define SD struct _instance_data

static ssize_t
gpio_driver_read(struct file *instance,
		 char __user *user, size_t count, loff_t *offset)
{
	unsigned long not_copied;
	unsigned long to_copy;
	char value;
	/*char command; */

	dev_info(drv_dev, "gpio_driver_read\n");

	/*
	   TODO: content:

	   command = 0x00;
	   i2c_master_send(slave, &command, 1);
	   i2c_master_recv(slave, &value, 1);
	*/

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
	unsigned char value;

	to_copy = min(count, sizeof(value));
	not_copied = copy_from_user(&value, user, to_copy);
	to_copy -= not_copied;

	i2c_master_send(slave, &value, sizeof(value));

	return to_copy;
}

static int
gpio_driver_open(struct inode *dev_node, struct file *instance)
{
	SD *data = NULL;

	int accmode = (instance->f_flags & O_ACCMODE);
	bool read_mode  = (accmode == O_RDONLY);
	bool write_mode = (accmode == O_WRONLY);

	if (!read_mode && !write_mode) {
		dev_err(drv_dev, "only O_RDONLY or O_WRONLY allowed\n");
		return -EIO;
	}

	if (write_mode)
		pr_info("write mode");

	if (read_mode)
		pr_info("read mode");

	data = (SD *) kmalloc(sizeof(SD), GFP_USER);
	if (data == NULL) {
		dev_err(drv_dev, "kmalloc\n");
		return -EIO;
	}
	memset(data, 0, sizeof(SD));

	if (write_mode) {
		data->direction = WRITE_PORT;
		pr_info("write mode");
	}

	if (read_mode) {
		data->direction = READ_PORT;
		pr_info("read mode");
	}

	instance->private_data = (void *) data;

	return 0;
}

static int
gpio_driver_close(struct inode *dev_node, struct file *instance)
{
	SD *data = NULL;

	if (instance->private_data) {
		data = (SD *) instance->private_data;

		if (data->slave)
			i2c_unregister_device(data->slave);

		/*
		 *  free i2c device
		 */

		kfree(instance->private_data);
	} else {
		dev_err(drv_dev, "close: instance->private_data == NULL\n");
	}

	return 0;
}

static long
gpio_driver_ioctl(struct file *instance, unsigned int cmd,
		  unsigned long __user arg)
{


	/*	adapter = i2c_get_adapter(1);
	if (adapter==NULL) {
		dev_err(drv_dev, "i2c_get_adapter\n");
		goto free_i2c_driver;
	}

	slave = i2c_new_device(adapter, &info_20);
	if (slave==NULL) {
		dev_err(drv_dev, "i2c_new_device\n");
		goto free_i2c_driver;
	}

	return 0;

free_i2c_driver:
	i2c_del_driver(&pcf8574_driver);

	dev_info(drv_dev, "close finished\n");

	return -EIO; */



	return 0;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.write= gpio_driver_write,
	.read = gpio_driver_read,
	.unlocked_ioctl = gpio_driver_ioctl,
	.open = gpio_driver_open,
	.release = gpio_driver_close
};

static int
pcf8574_probe(struct i2c_client *client,
	      const struct i2c_device_id *id)
{
	unsigned char value = 0x00;

	dev_info(drv_dev, "pcf8574_probe\n");

	pr_info("client %p\n", client);
	pr_info("client->addr %d", client->addr);
	pr_info("id %p\n", id);
	pr_info("id->name %s\n", id->name);

	if(client->addr != 0x26)
		pr_info("client->addr != 0x26\n");

	slave = client;

	i2c_master_recv(slave, &value, sizeof(value));

	pr_info("read %d\n", value);

	return 0;
}

static int
pcf8574_remove(struct i2c_client *client)
{
	dev_info(drv_dev, "pcf8574_remove\n");

	return 0;
}


static struct i2c_device_id pcf8574_idtable[] = {
        { "pcf8574", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcf8574_idtable);

static struct i2c_driver pcf8574_driver = {
        .driver = {
                .name   = "pcf8574",
        },
        .id_table       = pcf8574_idtable,
        .probe          = pcf8574_probe,
        .remove         = pcf8574_remove,
};

static int __init
i2c_gpio_driver_simple_init(void)
{
	pr_info("char_driver_init called\n");

	if (alloc_chrdev_region(&pcf8574_dev_number, 0, 1, DRIVER_NAME) < 0)
		return -EIO;

	pcf8574_object = cdev_alloc();
	if (pcf8574_object == NULL) {
		dev_err(drv_dev, "cdev_alloc\n");
		goto free_dev_number;
	}

	pcf8574_object->owner = THIS_MODULE;
	pcf8574_object->ops = &fops;

	if (cdev_add(pcf8574_object, pcf8574_dev_number, 1)) {
		dev_err(drv_dev, "cdev_add\n");
		goto free_cdev;
	}

	pcf8574_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(pcf8574_class)) {
		dev_err(drv_dev, "class_create\n");
		goto free_cdev;
	}

	drv_dev = device_create(pcf8574_class, NULL, pcf8574_dev_number,
				NULL, "%s", DRIVER_NAME);
	if (IS_ERR(drv_dev)) {
		dev_err(drv_dev, "device_create\n");
		goto free_class;
	}

	if (i2c_add_driver(&pcf8574_driver)) {
		dev_err(drv_dev, "i2c_add_driver\n");
		goto free_class;
	}

	return 0;

free_class:
	device_destroy(pcf8574_class, pcf8574_dev_number);
	class_destroy(pcf8574_class );

free_cdev:
	kobject_put(&pcf8574_object->kobj);

free_dev_number:
	unregister_chrdev_region(pcf8574_dev_number, 1);

	return -EIO;
}

static void __exit
i2c_gpio_driver_simple_exit(void)
{
	device_destroy(pcf8574_class, pcf8574_dev_number);
	class_destroy(pcf8574_class );

	cdev_del(pcf8574_object);
	unregister_chrdev_region(pcf8574_dev_number, 1);

	/*
	 * Has to be done by close:
	 * i2c_unregister_device(slave);
	 */
	i2c_del_driver(&pcf8574_driver);

	return;
}

module_init(i2c_gpio_driver_simple_init);
module_exit(i2c_gpio_driver_simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pcf8574 gpio simple - simple template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
