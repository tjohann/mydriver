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

#define DRIVER_NAME "char_driver_old_school" 

static dev_t dev_number;
static struct cdev *dev_object;
struct class *dev_class;

static struct file_operations fops = {
	/* 
	 * the supported syscalls 
	 */
};


static int __init
char_driver_old_school_init(void)
{
	/* get a device number */
	if (alloc_chrdev_region(&dev_number, 0, 1, DRIVER_NAME) < 0)
		return -EIO;

	dev_object = cdev_alloc();
	if (dev_object == NULL) 
		goto free_dev_number;

	dev_object->owner = THIS_MODULE;
	dev_object->ops = &fops;

	if (cdev_add(dev_object, dev_number, 1))
		goto free_cdev;

	/* add sysfs/udev entry */
	dev_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(dev_class))
		goto free_cdev;
	
	device_create(dev_class, NULL, dev_number, NULL, "%s", DRIVER_NAME);

	return 0;

free_cdev:
	kobject_put(&dev_object->kobj);
	
free_dev_number:
	unregister_chrdev_region(dev_number, 1);
	
	return -EIO;
}

static void __exit
char_driver_old_school_exit(void)
{
	device_destroy(dev_class, dev_number);
	class_destroy(dev_class );
	
	cdev_del(dev_object);
	unregister_chrdev_region(dev_number, 1);
	
	return;
}

module_init(char_driver_old_school_init);
module_exit(char_driver_old_school_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple char_old_school - template driver");
MODULE_AUTHOR("Thorsten Johannvorderbrueggen <thorsten.johannvorderbrueggen@t-online.de>");
