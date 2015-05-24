/*******************************************************************************/
/* mydriver.c -> simple driver for playing                                     */
/*                                                                             */
/* GPL                                                                         */
/* (c) 2013, thorsten.johannvorderbrueggen@t-online.de                         */
/*                                                                             */
/* This program is free software; you can redistribute it and/or modify        */
/* it under the terms of the GNU General Public License as published by        */
/* the Free Software Foundation; either version 2 of the License, or           */
/* (at your option) any later version.                                         */
/*                                                                             */
/* This program is distributed in the hope that it will be useful,             */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of              */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                */
/* GNU General Public License for more details.                                */
/*                                                                             */
/* You should have received a copy of the GNU General Public License           */
/* along with this program; if not, write to the Free Software                 */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA   */
/*                                                                             */
/*******************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DRIVER_NAME "mydriver" /* Name des Moduls */

static dev_t dev_number;
static struct cdev *driver_object;
struct class *template_class;

static struct file_operations fops = {
	/* 
	 * the supported syscalls 
	 */
};


/* --------------------------------------------------------------------------- */
/*                                init method                                  */
/* --------------------------------------------------------------------------- */
static int __init mydriver_init(void) {

/*
 * - first get a device number
 * - alloc a driver object
 * - init driver object
 * - add driver object to kernel
 *
 * - create sysfs entry (/sys/devices/virtual/mydriver/mydriver)
 * - create /dev entry via udev (/etc/udev/rules/90-<modulename>.rules)
 *   entry -> KERNEL=="mydriver", MODE="0666"
 */
	// get a device number
	if (alloc_chrdev_region(&dev_number, 0, 1, DRIVER_NAME) < 0)
		return -EIO;

	// alloc driver object
	if ( (driver_object = cdev_alloc()) == NULL) 
		goto free_device_number;

	// init driver object
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;

	// add driver object to kernel
	if (cdev_add(driver_object, dev_number, 1))
		goto free_cdev;

	// add sysfs/udev entry
	template_class = class_create(THIS_MODULE, DRIVER_NAME);
	device_create(template_class, 
		      NULL, 
		      dev_number,
		      NULL, 
		      "%s", 
		      DRIVER_NAME );
	return 0;

free_cdev:
	// kobject_put works similiar to cdel_del
	kobject_put(&driver_object->kobj); 
free_device_number:
	unregister_chrdev_region(dev_number, 
				 1);
	return -EIO;
}


/* --------------------------------------------------------------------------- */
/*                                exit method                                  */
/* --------------------------------------------------------------------------- */
static void __exit mydriver_exit( void )
{
	// delete /dev entry and remove sysfs entry
	device_destroy( template_class, dev_number );
	class_destroy( template_class );
	
        // free_cdev
	cdev_del( driver_object );
	// free_device_number
	unregister_chrdev_region( dev_number, 1 );
	return;
}

module_init(mydriver_init);
module_exit(mydriver_exit);
MODULE_LICENSE("GPL");
