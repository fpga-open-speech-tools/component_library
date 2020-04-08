// SPDX-License-Identifier: GPL-2.0+
/** @file FE_DPRAM.c
    
    This kernel driver controls a Dual Port RAM block
    
    @author Joshua Harthan (based on code written by Tyler Davis)
    @date 2020
    @copyright 2020 FlatEarth Inc, Bozeman MT
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include "custom_functions.h" // fixed point operations

/* Header for successful compilation */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joshua Harthan");
MODULE_DESCRIPTION("Loadable kernel module for Dual Port RAM");
MODULE_VERSION("1.0");

/* Structure necessary for fixed point number reperesentation */
struct fixed_num {
  int integer;
  int fraction;
  int fraction_len;
};

/* Register offsets of the DPRAM component */
#define CONTROL_ADDR_OFFSET 0x0
#define CONTROL_WR_OFFSET 0x1
#define CONTROL_DATA_OFFSET 0x2
#define REGISTER_WR_DOUT_OFFSET 0x3
#define LED_OUT_OFFSET 0x4

/* Driver function prototypes */
static int DPRAM_probe		(struct platform_device *pdev);
static int DPRAM_remove		(struct platform_device *pdev);
static ssize_t DPRAM_read	(struct file *filp, char *buffer, size_t len, loff_t *offset);
static ssize_t DPRAM_write	(struct file *filp, const char *buffer, size_t len, loff_t *offset);
static int DPRAM_open		(struct inode *inode, struct file *file);
static int DPRAM_release	(struct inode *inode, struct file *file);
static long DPRAM_ioctl		(struct file *filp, unsigned int cmd, unsigned long arg);

/* FPGA device functions */
static ssize_t control_addr_write    (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t control_addr_read     (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t control_wr_write      (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t control_wr_read       (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t control_data_write    (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t control_data_read     (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register_wr_dout_read (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t name_read	     (struct device *dev, struct device_attribute *attr, char *buf);

/* Device attributes that show up in /sys/class for device */
DEVICE_ATTR (control_address, 	0664,	control_addr_read, 	control_addr_write); 
DEVICE_ATTR (control_wr_en,    	0664,   control_wr_read,    	control_wr_write);
DEVICE_ATTR (control_data,      0664,   control_data_read,      control_data_write);
DEVICE_ATTR (register_wr_dout,  0444,   register_wr_dout_read,  NULL);
DEVICE_ATTR (name, 		0444, 	name_read, 		NULL);

/* Device struct */
struct fe_DPRAM_dev {
  struct cdev cdev;
  char *name;
  void __iomem *regs;
  int control_address;
  int control_wr_en;
  int control_data;
  int register_wr_dout
};
typedef struct fe_DPRAM_dev fe_DPRAM_dev_t;

/* Global Variables */
static struct class *cl;  // Global variable for the device class
static dev_t dev_num;

/* ID Matching struct to match driver to tree */
static struct of_device_id fe_DPRAM_dt_ids[] = {
  {
    .compatible = "dev,fe-dpram"
  },
  { }
};
MODULE_DEVICE_TABLE(of, fe_DPRAM_dt_ids);

/* Platform driver struct */
static struct platform_driver DPRAM_platform = {
  .probe = DPRAM_probe,
  .remove = DPRAM_remove,
  .driver = {
    .name = "Flat Earth Dual Port RAM Driver",
    .owner = THIS_MODULE,
    .of_match_table = fe_DPRAM_dt_ids
  }
};

/* File ops struct */
static const struct file_operations fe_DPRAM_fops = {
  .owner = THIS_MODULE,
  .read = DPRAM_read,
  .write = DPRAM_write,
  .open = DPRAM_open,
  .release = DPRAM_release,
  .unlocked_ioctl = DPRAM_ioctl,
};

/* Initialize device driver */
static int DPRAM_init(void) {
  int ret_val = 0;

  pr_info("Initializing the Flat Earth Dual Port RAM module...\n");

  pr_info("Probe function start...\n");

  // Register our driver with the "Platform Driver" bus
  ret_val = platform_driver_register(&DPRAM_platform);  
  if (ret_val != 0) {
    pr_err("platform_driver_register returned %d\n", ret_val);
    return ret_val;
  }

   char* argv[] = {"/bin/bash", "/root/DPRAM/DPRAM.sh", NULL};

   static char* envp[] = { "HOME=/",  "TERM=linux",   "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
   call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
 
  pr_info("Flat Earth Dual Port RAM module successfully initialized!\n");
  
  return 0;
}

/* Probe function of device driver */
static int DPRAM_probe(struct platform_device *pdev) {
  
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
  
  platform_set_drvdata(pdev, (void *)fe_DPRAM_devp);
  fe_DPRAM_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
  if (fe_DPRAM_devp->name == NULL)
    goto bad_mem_alloc;
  
  strcpy(fe_DPRAM_devp->name, (char *)pdev->name);
  pr_info("%s\n", (char *)pdev->name);
  status = alloc_chrdev_region(&dev_num, 0, 1, "fe_DPRAM_");
  if (status != 0)
    goto bad_alloc_chrdev_region;
  
  sprintf(deviceMinor, "%d", MAJOR(dev_num));
  strcat(deviceName, deviceMinor);
  pr_info("%s\n", deviceName);
  cl = class_create(THIS_MODULE, deviceName);
  if (cl == NULL)
    goto bad_class_create;
  
  cdev_init(&fe_DPRAM_devp->cdev, &fe_DPRAM_fops);
  status = cdev_add(&fe_DPRAM_devp->cdev, dev_num, 1);
  if (status != 0)
    goto bad_cdev_add;
  
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

  status = device_create_file(device_obj, &dev_attr_register_wr_dout);
  if (status)
          goto bad_device_create_file_4;

  status = device_create_file(device_obj, &dev_attr_name);
  if (status)
    goto bad_device_create_file_5;

  pr_info("DPRAM_probe exit\n");
  return 0;

  // Error functions for probe function  
  bad_device_create_file_5:
    device_remove_file(device_obj, &dev_attr_name);
  
  bad_device_create_file_4:
    device_remove_file(device_obj, &dev_attr_register_wr_dout);

  bad_device_create_file_3:
    device_remove_file(device_obj, &dev_attr_control_data);

  bad_device_create_file_2:
    device_remove_file(device_obj, &dev_attr_control_wr_en);

  bad_device_create_file_1:
    device_remove_file(device_obj, &dev_attr_control_address);

  bad_device_create_file_0:
    device_destroy(cl, dev_num);

  bad_device_create:
    cdev_del(&fe_DPRAM_devp->cdev);

  bad_cdev_add:
    class_destroy(cl);

  bad_class_create:
    unregister_chrdev_region(dev_num, 1);

  bad_alloc_chrdev_region:

  bad_mem_alloc:

  bad_ioremap:
    ret_val = PTR_ERR(fe_DPRAM_devp->regs);

  bad_exit_return:
    pr_info("DPRAM_probe bad exit\n");
  
  return ret_val;
}

/* FPGA Attribute functions */
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

static ssize_t register_wr_dout_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register_wr_dout, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  sprintf(buf, "%s\n", devp->name);
  return strlen(buf);
}

/* Device driver functions */
static int DPRAM_open(struct inode *inode, struct file *file) {
  pr_info("DPRAM : Device OPEN\n"); // Test
  return 0;
}

static int DPRAM_release(struct inode *inode, struct file *file) {
  pr_info("DPRAM : Device RELEASE\n"); // Test
  return 0;
}

static long DPRAM_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
  pr_info("DPRAM : Device IOCTL\n"); // Test
  return 0;
}

static ssize_t DPRAM_read(struct file *filp, char *buffer, size_t len, loff_t *offset) {
  pr_info("DPRAM : Device READ\n"); // Test
  return 0;
}    

static ssize_t DPRAM_write(struct file *filp, const char *buffer, size_t len, loff_t *offset) {
  pr_info("DPRAM : Device WRITE\n"); // Test
  return 0;
}

static int DPRAM_remove(struct platform_device *pdev) {
 
  fe_DPRAM_dev_t *dev = (fe_DPRAM_dev_t *)platform_get_drvdata(pdev);
  
  pr_info("DPRAM_remove enter\n");
  
  cdev_del(&dev->cdev);
  unregister_chrdev_region(dev_num, 2);
  iounmap(dev->regs);
  
  pr_info("DPRAM_remove exit\n");
  
  return 0;
}

static void DPRAM_exit(void) {
  pr_info("Flat Earth Dual Port RAM module exit\n");
  platform_driver_unregister(&DPRAM_platform);
  pr_info("Flat Earth Dual Port RAM module successfully unregistered\n");
}

/* End of file */
module_init(DPRAM_init);
module_exit(DPRAM_exit);
