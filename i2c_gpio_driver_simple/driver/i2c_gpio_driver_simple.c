/*
 * i2c_gpio_driver_simple.c -> simple template driver
 *
 * GPL
 * (c) 2016-2017, thorsten.johannvorderbrueggen@t-online.de
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
#include <linux/i2c.h>
#include <linux/slab.h>

#include "../common.h"

/*
 * #define WRITE_PORT 0x01
 */

static int pcf8574_probe(struct i2c_client *client,
			const struct i2c_device_id *id);
static int pcf8574_remove(struct i2c_client *client);

static dev_t pcf8574_dev_number;
static struct cdev *pcf8574_object;
struct class *pcf8574_class;
static struct device *drv_dev;

struct _instance_data {
	struct i2c_client *slave;
	struct i2c_adapter *adapter;
	int adapter_nr;
	unsigned short addr;
};
#define SD struct _instance_data

static struct i2c_device_id pcf8574_idtable[] = {
        { "pcf8574_simple", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcf8574_idtable);

static struct i2c_driver pcf8574_driver = {
        .driver = {
                .name   = "pcf8574_simple",
        },
        .id_table       = pcf8574_idtable,
        .probe          = pcf8574_probe,
        .remove         = pcf8574_remove,
};


static ssize_t
gpio_driver_write(struct file *instance,
		  const char __user *user, size_t count, loff_t *offset)
{
	int not_send;
	unsigned char value;

	SD *data = NULL;

	if (!instance->private_data) {
		dev_err(drv_dev, "write: instance->private_data == NULL\n");
		return -EFAULT;
	}

	data = (SD *) instance->private_data;
	if (!data->slave) {
		dev_err(drv_dev, "no valid slave pointer\n");
		return -EFAULT;
	}

	if (get_user(value, (unsigned char __user *) user)) {
		dev_err(drv_dev, "could not copy from userspace\n");
		return -EFAULT;
	}

	dev_info(drv_dev, "got %d from userspace\n", value);

	not_send = i2c_master_send(data->slave, &value, sizeof(value));
	if (not_send < 0) {
		dev_err(drv_dev, "i2c_master_send\n");
		return -EFAULT;
	}

	return 0;
}

static int
gpio_driver_open(struct inode *dev_node, struct file *instance)
{
	SD *data = NULL;

	int accmode = (instance->f_flags & O_ACCMODE);
	bool write_mode = (accmode == O_WRONLY);

	if (!write_mode) {
		dev_err(drv_dev, "only O_WRONLY allowed\n");
		return -EIO;
	}

	data = (SD *) kmalloc(sizeof(SD), GFP_USER);
	if (data == NULL) {
		dev_err(drv_dev, "kmalloc\n");
		return -EIO;
	}
	memset(data, 0, sizeof(SD));
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

		kfree(instance->private_data);
	} else {
		dev_err(drv_dev, "close: instance->private_data == NULL\n");
		return -EFAULT;
	}

	return 0;
}

static long
gpio_driver_ioctl(struct file *instance, unsigned int cmd,
		  unsigned long __user arg)
{
	struct client_config __user user_data;
	SD *data = NULL;

	if (!instance->private_data) {
		dev_err(drv_dev, "ioctl: instance->private_data == NULL\n");
		return -1;
	}

	data = (SD *) instance->private_data;

	switch (cmd) {
	case IOCTL_SET_I2C_CLIENT:
		if (copy_from_user(&user_data,
					(struct client_config __user *) arg,
					sizeof(struct client_config)))
			return -EFAULT;

		dev_info(drv_dev, "addr: 0x%x\n", user_data.addr);
		dev_info(drv_dev, "adapter_nr: %d\n", user_data.adapter_nr);

		struct i2c_board_info info = {
			I2C_BOARD_INFO("pcf8574_simple", user_data.addr),
		};

		data->adapter = i2c_get_adapter(user_data.adapter_nr);
		if (data->adapter == NULL)
			goto error;

		data->slave = i2c_new_client_device(data->adapter, &info);
		if (data->slave == NULL)
			goto error;

		data->addr = user_data.addr;
		data->adapter_nr = user_data.adapter_nr;

		break;
	default:
		dev_err(drv_dev, "unknown ioctl 0x%x\n", cmd);
		return -EINVAL;
	}

	return 0;
error:
	dev_err(drv_dev, "i2c_new_device or i2c_get_adapter\n");
	i2c_del_driver(&pcf8574_driver);

	return -EFAULT;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.write= gpio_driver_write,
	.unlocked_ioctl = gpio_driver_ioctl,
	.open = gpio_driver_open,
	.release = gpio_driver_close
};

static int
pcf8574_probe(struct i2c_client *client,
	      const struct i2c_device_id *id)
{
	dev_info(drv_dev, "client %p\n", client);
	dev_info(drv_dev, "client->addr 0x%x", client->addr);
	dev_info(drv_dev, "id %p\n", id);
	dev_info(drv_dev, "id->name %s\n", id->name);

	return 0;
}

static int
pcf8574_remove(struct i2c_client *client)
{
	dev_info(drv_dev, "pcf8574_remove\n");
	return 0;
}

static int __init
i2c_gpio_driver_simple_init(void)
{
	pr_info("i2c_gpio_driver_simple_init called\n");

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

	i2c_del_driver(&pcf8574_driver);

	return;
}

module_init(i2c_gpio_driver_simple_init);
module_exit(i2c_gpio_driver_simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pcf8574 gpio simple - simple template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
