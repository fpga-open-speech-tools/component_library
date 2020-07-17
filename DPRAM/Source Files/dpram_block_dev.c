// SPDX-License-Identifier: GPL-2.0+
/** @file FE_DPRAM.c
    
    This kernel driver controls a Dual Port RAM block
    
    @author Joshua Harthan (based on code written by Tyler Davis)
    @date 2020
    @copyright 2020 FlatEarth Inc, Bozeman MT
*/

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include "custom_functions.h"

/* File header necessary for kernel compilation */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joshua Harthan");
MODULE_DESCRIPTION("Loadable kernel module for the Flat Earth Dual Port RAM component");
MODULE_VERSION("1.0");

/* Fixed point number structure */
struct fixed_num {
  int integer;
  int fraction;
  int fraction_len;
};

/* Global Variable Declaration  */
typedef struct fe_DPRAM_dev fe_DPRAM_dev_t;
static struct class *cl;  // Global variable for the device class
static dev_t dev_num;

/* Set the constant for the kernel sector size, major number and name, and minor numbers */
#define KERNEL_SECTOR_SIZE 512 // Indicates to the kernel the size of the kernel sector 
#define FE_DPRAM_MAJOR 0 // Major number of zero indicates that the kernel will dyanamically allocate the major number
#define FE_DPRAM_MINORS 1 // Minor number of one indicates the device is non-partitionable
#define MAJOR_NAME "DPRAM" // The major name to appear in list of devices

/* Register offsets of the DPRAM component */
#define CONTROL_ADDR_OFFSET 0x0
#define CONTROL_WR_OFFSET 0x1
#define CONTROL_DATA_OFFSET 0x2
#define REGISTER_WR_DOUT_OFFSET 0x3
#define LED_OUT_OFFSET 0x4

/* Function Prototypes for driver disk initialization */
static int fe_DPRAM_probe	(struct platform_device *pdev);
static int fe_DPRAM_remove	(struct platform_device *pdev);
static void fe_DPRAM_request	(struct request_queue *q);
static void fe_DPRAM_transfer	(struct fe_DPRAM_dev *dev, unsigned long sector, unsigned long nsect, char *buffer, int write); // transfer data from buffer to request queue
static int fe_DPRAM_open	(struct block_device *bdev, fmode_t mode); // lock the device before writing/reading data
static void fe_DPRAM_release	(struct gendisk *gd, fmode_t mode); // unlock the device before continuing operations
static int fe_DPRAM_ioctl	(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg); // I/O control for DPRAM component 

/* FPGA device functions */
static ssize_t control_addr_write    (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t control_addr_read     (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t control_wr_write      (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t control_wr_read       (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t control_data_write    (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t control_data_read     (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t name_read             (struct device *dev, struct device_attribute *attr, char *buf);

/* Device attributes that show up in /sys/class for device */
DEVICE_ATTR (control_address,   0644,   control_addr_read,      control_addr_write);
DEVICE_ATTR (control_wr_en,     0644,   control_wr_read,        control_wr_write);
DEVICE_ATTR (control_data,      0664,   control_data_read,      control_data_write);
DEVICE_ATTR (name,              0444,   name_read,              NULL);

/* Module parameters for the block driver */
static int major_num = 0;
module_param(major_num, int, 0444);
static int hardsect_size = 512;
module_param(hardsect_size, int, 0444);
static int nsectors = 16384;
module_param(nsectors, int, 0444);

/* ID Matching struct to match driver to tree */
static struct of_device_id fe_DPRAM_dt_ids[] = {
  {
    .compatible = "dev,fe-dpram"
  },
  { }
};
MODULE_DEVICE_TABLE(of, fe_DPRAM_dt_ids);

/* Device structure */
typedef struct fe_DPRAM_dev fe_DPRAM_dev_t;
struct fe_DPRAM_dev {
  int size; 			// device size in vectors
  u8 *data; 			// the data array 
  short users;			// # of users
  spinlock_t lock;		// mutual exclusion
  struct request_queue *queue;	// device request queue	
  struct gendisk *gd;		// gendisk struct
  struct timer_list timer;      // media change timer
  void __iomem *regs;             // pointer to the device's register
  int control_address;	
  int control_wr_en;
  int control_data;
} device;

/* Platform driver struct */
static struct platform_driver DPRAM_platform = {
  .probe =  fe_DPRAM_probe,
  .remove = fe_DPRAM_remove,
  .driver = {
    .name = "Flat Earth Dual Port RAM Driver",
    .owner = THIS_MODULE,
    .of_match_table = fe_DPRAM_dt_ids
  }
};

/* Block dev ops struct */
static const struct block_device_operations fe_DPRAM_ops = {
   .owner = THIS_MODULE,
   .open = fe_DPRAM_open,
   .release = fe_DPRAM_release,
   .ioctl = fe_DPRAM_ioctl
};

/* Initialize the component */
static int fe_DPRAM_init(void){
  int status;  
  int ret_val = 0;

  pr_info("\n\nInitializing the Flat Earth DPRAM module...\n");

  pr_info("Register our driver with the Platform Driver Bus...\n");

  // Register our driver with the "Platform Driver" bus
  ret_val = platform_driver_register(&DPRAM_platform);
  if (ret_val != 0) {
    pr_err("platform_driver_register returned %d\n", ret_val);
    return ret_val;
  }

  pr_info("Platform Driver Bus successfully registered!\n");
 
  // Register major number to the block device
  status = register_blkdev(FE_DPRAM_MAJOR, MAJOR_NAME);
  if (FE_DPRAM_MAJOR < 0) {
    printk(KERN_ERR "fe-DPRAM : Unable to register fe_DPRAM device.\n");
    return -EBUSY;
  };
  pr_info("The MAJOR has been successfully registered for the FE DPRAM component!\n");

  pr_info("Begin DPRAM device block creation:\n");

  device.size = nsectors*hardsect_size;
  spin_lock_init(&device.lock); // Initialize the device lock
  pr_info("The size of the device is %d sectors long.\n", device.size);

  device.data = vmalloc(device.size);
  pr_info("The memory for this device has been allocated successfully with vmalloc!\n");

  if (device.data == NULL){
    printk(KERN_NOTICE "vmalloc failure\n");
    return -ENOMEM;
  }

  pr_info("Device structute successfully initialized!\n");

  /* Create the request queue */
  device.queue = blk_init_queue(fe_DPRAM_request, &device.lock); // fe_DPRAM_request is the request function (that which performs read and write requests); blk_init_queue can fail allocation of memory so caution here
  if(device.queue == NULL){
    return -ENOMEM;
  }
  blk_queue_logical_block_size(device.queue, KERNEL_SECTOR_SIZE);
  device.queue->queuedata = device.queue;
  pr_info("Request queue successfully initialized!\n");

  /* Initialize the gendisk structure */
  device.gd = alloc_disk(FE_DPRAM_MINORS);
  if (!device.gd){
    printk(KERN_NOTICE "alloc_disk failure\n");
    unregister_blkdev(FE_DPRAM_MAJOR, MAJOR_NAME);
    if(device.data){
      vfree(device.data);
      pr_info("The data within the device has been deleted.");
    }
  }
  device.gd->major = FE_DPRAM_MAJOR; // Set the major number of the device        
  device.gd->first_minor = 0; // FE_DPRAM_MINORS is the number of each minors our device can support; 0 would indicate the device as non-partitionable
  device.gd->fops = &fe_DPRAM_ops; // Pointer to the device block_device_operations structure of the device
  device.gd->queue = device.queue; // Put the gendisk queue as the device queue
  device.gd->private_data = &device; // Pointer to gendisk specific data structure
  snprintf(device.gd->disk_name, 32, "fe_DPRAM"); // print the gendisk name to buffer
  set_capacity(device.gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE)); // KERNEL_SECTOR_SIZE is a locally defined constant as the kernel's 512-byte sectors      

  pr_info("Gendisk structure successfully initialized!\n");

  add_disk(device.gd); // Add the gendisk structure 

  pr_info("Flat Earth DPRAM module successfully initialized!\n");
  
  return 0;
};

/* Data transfer function */
static void fe_DPRAM_transfer (struct fe_DPRAM_dev *dev, unsigned long sector, unsigned long nsect, char *buffer, int write){
  unsigned long offset = sector*KERNEL_SECTOR_SIZE;
  unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;

  pr_info("The driver has entered the transfer function.\n");

  if((offset + nbytes) > device.size){
    printk(KERN_NOTICE "Beyond-end write (%ld %ld)\n",offset, nbytes);
    return;
  };
  if (write)
    memcpy(device.data + offset, buffer, nbytes);
  else
    memcpy(buffer, device.data + offset, nbytes);    

  pr_info("The driver has exited the transfer function.\n");
};

/* Request Function */
static void fe_DPRAM_request(struct request_queue *q){
  struct request *req;

  pr_info("The driver has entered the request function.\n");

  while (1) {
    req = blk_fetch_request(q);
    if(req == NULL){
      break;
    }
    if(blk_rq_is_passthrough(req)){
      printk(KERN_NOTICE "Skip non-fs request\n");
      __blk_end_request_all(req, -EIO);
      continue;
    }
    fe_DPRAM_transfer(&device, req->__sector, req->__data_len, bio_data(req->bio), rq_data_dir(req));
    __blk_end_request_cur(req, 0);
  }
  pr_info("The driver has exited the request function.\n");
};

/* Open Method */
static int fe_DPRAM_open(struct block_device *bdev, fmode_t mode){
    
  pr_info("The driver has entered the open function.\n");

  del_timer_sync(&device.timer); // once called, delete the 30 sec media removal timer 
  spin_lock(&device.lock); //lock the spinlock AFTER the timer is deleted
  if(!device.users){
    check_disk_change(bdev); // check if a media change has occured
  }
  device.users++; // increment the user count to show how many users are in use of driver
  spin_unlock(&device.lock);
    
  pr_info("The drive has exited the open function.\n");
    
  return 0;
};

/* Release Method */
static void fe_DPRAM_release(struct gendisk *gd, fmode_t mode){
    
  pr_info("The driver has entered the release function.\n");
    
  spin_lock(&device.lock);
  device.users--; // decrement the user count to update the number of users using the device
  spin_unlock(&device.lock);

  pr_info("The driver has exited the release function.\n");

  return; 
};

/* Generic I/O Control Method */
static int fe_DPRAM_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long args){
  long size;
  struct hd_geometry geo;

  pr_info("The driver has entered the IOCTL function.\n");
    
  // Get geometry of the device, arbitrary values input
  switch(cmd){
    case HDIO_GETGEO:
      size = device.size*(hardsect_size/KERNEL_SECTOR_SIZE);
      geo.cylinders = (size & ~0x3f) >> 6; //size of device AND w/ 00111111, right shift by 6
      geo.heads = 2;
      geo.sectors = 16; 
      geo.start = 0;
      if (copy_to_user((void __user *) args, &geo, sizeof(geo))){
        return -EFAULT;
      }
    return 0;
   }

  pr_info("The driver has exited the IOCTL function.\n");

  return -ENOTTY; // unknown command flag thrown
};


/* Probe function of device driver */
static int fe_DPRAM_probe(struct platform_device *pdev) {

  int ret_val = -EBUSY;

  char deviceName[20] = "fe_DPRAM_";
  char deviceMinor[20];
  int status;

  struct device *device_obj;
  fe_DPRAM_dev_t * fe_DPRAM_devp;

  pr_info("DPRAM_probe enter...\n");

  struct resource *r = 0;
  r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (r == NULL) {
    pr_err("IORESOURCE_MEM (register space) does not exist\n");
    goto bad_exit_return;  }

  // Create structure to hold device information
  fe_DPRAM_devp = devm_kzalloc(&pdev->dev, sizeof(fe_DPRAM_dev_t), GFP_KERNEL);
  fe_DPRAM_devp->regs = devm_ioremap_resource(&pdev->dev, r);
  if (IS_ERR(fe_DPRAM_devp->regs))
    goto bad_ioremap;

  sprintf(deviceMinor, "%d", MAJOR(dev_num));
  strcat(deviceName, deviceMinor);
  pr_info("%s\n", deviceName);
  cl = class_create(THIS_MODULE, deviceName);
  if (cl == NULL)
    goto bad_class_create;

  device_obj = device_create(cl, NULL, dev_num, NULL, deviceName);
  if (device_obj == NULL)
    goto bad_device_create;

  dev_set_drvdata(device_obj, fe_DPRAM_devp);

  // Check each register value, upon initialization reset the component
  status = device_create_file(device_obj, &dev_attr_control_address);
  if (status)
    goto bad_device_create_file_1;

  status = device_create_file(device_obj, &dev_attr_control_wr_en);
  if (status)
    goto bad_device_create_file_2;

  status = device_create_file(device_obj, &dev_attr_control_data);
  if (status)
    goto bad_device_create_file_3;

  status = device_create_file(device_obj, &dev_attr_name);
  if (status)
    goto bad_device_create_file_4;

  pr_info("DPRAM_probe exit\n");
  return 0;

  // Error functions for probe function
  bad_device_create_file_4:
    device_remove_file(device_obj, &dev_attr_name);

  bad_device_create_file_3:
    device_remove_file(device_obj, &dev_attr_control_data);

  bad_device_create_file_2:
    device_remove_file(device_obj, &dev_attr_control_wr_en);

  bad_device_create_file_1:
    device_remove_file(device_obj, &dev_attr_control_address);

  bad_device_create_file_0:
    device_destroy(cl, dev_num);

  bad_device_create:

  bad_class_create:
    unregister_chrdev_region(dev_num, 1);

  bad_mem_alloc:

  bad_ioremap:
    ret_val = PTR_ERR(fe_DPRAM_devp->regs);

  bad_exit_return:
    pr_info("DPRAM_probe bad exit\n");

  return ret_val;
}

/* FPGA functions */
static ssize_t control_addr_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->control_address, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t control_addr_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->control_address = tempValue;
  iowrite32(devp->control_address, (u32 *)devp->regs + 0);
  return count;
}

static ssize_t control_wr_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->control_wr_en, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t control_wr_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->control_wr_en = tempValue;
  iowrite32(devp->control_wr_en, (u32 *)devp->regs + 1);
  return count;
}

static ssize_t control_data_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->control_data, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t control_data_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->control_data = tempValue;
  iowrite32(devp->control_data, (u32 *)devp->regs + 2);
  return count;
}

static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  sprintf(buf, "%s\n", devp->gd->disk_name);
  return strlen(buf);
}

static int fe_DPRAM_remove(struct platform_device *pdev) {

  fe_DPRAM_dev_t *dev = (fe_DPRAM_dev_t *)platform_get_drvdata(pdev);

  pr_info("DPRAM_remove enter\n");

  del_gendisk(device.gd); // delete the gendisk, clean up any partitioning information
  put_disk(device.gd); // relsease the gendisk object reference
  unregister_blkdev(FE_DPRAM_MAJOR, MAJOR_NAME); // unregister the block so it no longer takes up a major
  blk_cleanup_queue(device.queue); // delete any request in the device's queue
  vfree(device.data); // free memory space
  
  unregister_chrdev_region(dev_num, 2);
  iounmap(dev->regs);

  pr_info("DPRAM_remove exit\n");

  return 0;
}

/* Exit Function, remove the device */
static void __exit fe_DPRAM_exit(void) {
  pr_info("Flat Earth DPRAM module exit.\n");
  platform_driver_unregister(&DPRAM_platform);
  pr_info("Flat Earth DPRAM module successfully unregistered!\n");
};

/* End of file */
module_init(fe_DPRAM_init);
module_exit(fe_DPRAM_exit);
