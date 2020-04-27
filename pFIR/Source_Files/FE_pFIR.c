// SPDX-License-Identifier: GPL-2.0+
/** @file FE_pFIR.c
    
    This kernel driver controls a programmable FIR filter block
    
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
MODULE_DESCRIPTION("Loadable kernel module for a programmable FIR filter");
MODULE_VERSION("1.0");

/* Structure necessary for fixed point number reperesentation */
struct fixed_num {
  int integer;
  int fraction;
  int fraction_len;
};

/* Register offsets of the pFIR component */
#define ENABLE_OFFSET  0x0
#define RW_ADDR_OFFSET 0x1
#define WR_DATA_OFFSET 0x2
#define WR_EN_OFFSET   0x3

/* Driver function prototypes */
static int pFIR_probe		(struct platform_device *pdev);
static int pFIR_remove		(struct platform_device *pdev);
static ssize_t pFIR_read	(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t pFIR_write	(struct file *file, const char *buffer, size_t len, loff_t *offset);
static int pFIR_open		(struct inode *inode, struct file *file);
static int pFIR_release		(struct inode *inode, struct file *file);
static long pFIR_ioctl		(struct file *file, unsigned int cmd, unsigned long arg);

/* FPGA device functions */
static ssize_t enable_write     (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t enable_read      (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t rw_addr_write    (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t rw_addr_read     (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t wr_data_write    (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t wr_data_read     (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t wr_en_read       (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t wr_en_write      (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t name_read        (struct device *dev, struct device_attribute *attr, char *buf);

/* Device attributes that show up in /sys/class for device */
DEVICE_ATTR (enable, 		0664,   enable_read,		enable_write);
DEVICE_ATTR (rw_addr,           0664,   rw_addr_read,           rw_addr_write);
DEVICE_ATTR (wr_data,		0644,   wr_data_read, 		wr_data_write);
DEVICE_ATTR (wr_en,             0664,   wr_en_read,      	wr_en_write);
DEVICE_ATTR (name, 		0444, 	name_read, 		NULL);

/* Device struct */
struct fe_pFIR_dev {
  struct cdev cdev;
  char *name;
  void __iomem *regs;
  int enable;
  int rw_addr;
  int wr_data;
  int wr_en;
};
typedef struct fe_pFIR_dev fe_pFIR_dev_t;

/* Global Variables */
static struct class *cl;  // Global variable for the device class
static dev_t dev_num;

/* ID Matching struct to match driver to tree */
static struct of_device_id fe_pFIR_dt_ids[] = {
  {
    .compatible = "dev,fe-pFIR_Testing"
  },
  { }
};
MODULE_DEVICE_TABLE(of, fe_pFIR_dt_ids);

/* Platform driver struct */
static struct platform_driver pFIR_platform = {
  .probe = pFIR_probe,
  .remove = pFIR_remove,
  .driver = {
    .name = "Flat Earth Programmable FIR Filter Driver",
    .owner = THIS_MODULE,
    .of_match_table = fe_pFIR_dt_ids
  }
};

/* File ops struct */
static const struct file_operations fe_pFIR_fops = {
  .owner = THIS_MODULE,
  .read = pFIR_read,
  .write = pFIR_write,
  .open = pFIR_open,
  .release = pFIR_release,
  .unlocked_ioctl = pFIR_ioctl,
};

/* Initialize device driver */
static int pFIR_init(void) {
  int ret_val = 0;

  pr_info("Initializing the Flat Earth Programmable FIR Filter module...\n");

  pr_info("Probe function start...\n");

  // Register our driver with the "Platform Driver" bus
  ret_val = platform_driver_register(&pFIR_platform);  
  if (ret_val != 0) {
    pr_err("platform_driver_register returned %d\n", ret_val);
    return ret_val;
  }

  pr_info("Flat Earth Programmable FIR Filter module successfully initialized!\n");
  
  return 0;
}

/* Probe function of device driver */
static int pFIR_probe(struct platform_device *pdev) {
  
  int ret_val = -EBUSY;

  char deviceName[20] = "fe_pFIR_";
  char deviceMinor[20];
  int status;
 
  struct device *device_obj;
  fe_pFIR_dev_t * fe_pFIR_devp;
 
  pr_info("pFIR_probe enter...\n");
 
  struct resource *r = 0;
  r = platform_get_resource(pdev, IORESOURCE_MEM, 0); 
  if (r == NULL) {
    pr_err("IORESOURCE_MEM (register space) does not exist\n");
    goto bad_exit_return;  }
 
  // Create structure to hold device information
  fe_pFIR_devp = devm_kzalloc(&pdev->dev, sizeof(fe_pFIR_dev_t), GFP_KERNEL);
  fe_pFIR_devp->regs = devm_ioremap_resource(&pdev->dev, r);
  if (IS_ERR(fe_pFIR_devp->regs))
    goto bad_ioremap;
  
  platform_set_drvdata(pdev, (void *)fe_pFIR_devp);
  fe_pFIR_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
  if (fe_pFIR_devp->name == NULL)
    goto bad_mem_alloc;
  
  strcpy(fe_pFIR_devp->name, (char *)pdev->name);
  pr_info("%s\n", (char *)pdev->name);
  status = alloc_chrdev_region(&dev_num, 0, 1, "fe_pFIR_");
  if (status != 0)
    goto bad_alloc_chrdev_region;
  
  sprintf(deviceMinor, "%d", MAJOR(dev_num));
  strcat(deviceName, deviceMinor);
  pr_info("%s\n", deviceName);
  cl = class_create(THIS_MODULE, deviceName);
  if (cl == NULL)
    goto bad_class_create;
  
  cdev_init(&fe_pFIR_devp->cdev, &fe_pFIR_fops);
  status = cdev_add(&fe_pFIR_devp->cdev, dev_num, 1);
  if (status != 0)
    goto bad_cdev_add;
  
  device_obj = device_create(cl, NULL, dev_num, NULL, deviceName);
  if (device_obj == NULL)
    goto bad_device_create;

  dev_set_drvdata(device_obj, fe_pFIR_devp);  

  // Check each register value, upon initialization reset the component
  status = device_create_file(device_obj, &dev_attr_enable);
  if (status)
    goto bad_device_create_file_1;
  
  status = device_create_file(device_obj, &dev_attr_rw_addr);
  if (status)
    goto bad_device_create_file_2;

  status = device_create_file(device_obj, &dev_attr_wr_data);
  if (status)
    goto bad_device_create_file_3;

  status = device_create_file(device_obj, &dev_attr_wr_en);
  if (status)
    goto bad_device_create_file_4;

  status = device_create_file(device_obj, &dev_attr_name);
  if (status)
    goto bad_device_create_file_5;

  pr_info("pFIR_probe exit\n");
  return 0;

  // Error functions for probe function  
  bad_device_create_file_5:
    device_remove_file(device_obj, &dev_attr_name);
  
  bad_device_create_file_4:
    device_remove_file(device_obj, &dev_attr_wr_en);
  
  bad_device_create_file_3:
    device_remove_file(device_obj, &dev_attr_wr_data);

  bad_device_create_file_2:
    device_remove_file(device_obj, &dev_attr_rw_addr);

  bad_device_create_file_1:
    device_remove_file(device_obj, &dev_attr_enable);

  bad_device_create_file_0:
    device_destroy(cl, dev_num);

  bad_device_create:
    cdev_del(&fe_pFIR_devp->cdev);

  bad_cdev_add:
    class_destroy(cl);

  bad_class_create:
    unregister_chrdev_region(dev_num, 1);

  bad_alloc_chrdev_region:

  bad_mem_alloc:

  bad_ioremap:
    ret_val = PTR_ERR(fe_pFIR_devp->regs);

  bad_exit_return:
    pr_info("pFIR_probe bad exit\n");
  
  return ret_val;
}

/* FPGA Attribute functions */
static ssize_t enable_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_pFIR_dev_t * devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->enable, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t enable_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_pFIR_dev_t *devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->enable = tempValue;
  iowrite32(devp->enable, (u32 *)devp->regs + 0);
  return count;
}


static ssize_t rw_addr_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_pFIR_dev_t * devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->rw_addr, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t rw_addr_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_pFIR_dev_t *devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->rw_addr = tempValue;
  iowrite32(devp->rw_addr, (u32 *)devp->regs + 1);
  return count;
}

static ssize_t wr_data_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_pFIR_dev_t * devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->wr_data, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t wr_data_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_pFIR_dev_t *devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->wr_data = tempValue;
  iowrite32(devp->wr_data, (u32 *)devp->regs + 2);
  return count;
}

static ssize_t wr_en_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_pFIR_dev_t * devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->wr_en, 0, true, 9);
  strcat2(buf,"\n");
  return strlen(buf);
}

static ssize_t wr_en_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_pFIR_dev_t *devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count ++;
    }
  }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->wr_en = tempValue;
  iowrite32(devp->wr_en, (u32 *)devp->regs + 3);
  return count;
}

static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_pFIR_dev_t *devp = (fe_pFIR_dev_t *)dev_get_drvdata(dev);
  sprintf(buf, "%s\n", devp->name);
  return strlen(buf);
}

/* Device driver functions */
static int pFIR_open(struct inode *inode, struct file *file) {
  pr_info("pFIR : Device OPEN\n"); // Test
  return 0;
}

static int pFIR_release(struct inode *inode, struct file *file) {
  pr_info("pFIR : Device RELEASED\n"); // Test
  return 0;
}

static long pFIR_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
  pr_info("pFIR : Device IOCTL\n"); // Test
  return 0;
}

static ssize_t pFIR_read(struct file *filp, char *buffer, size_t len, loff_t *offset) {
  pr_info("pFIR : Device READ\n"); // Test
  return 0;
}    

static ssize_t pFIR_write(struct file *file, const char *buffer, size_t len, loff_t *offset) {
  pr_info("pFIR : Device WRITE\n"); // Test
  return 0;
}

static int pFIR_remove(struct platform_device *pdev) {
 
  fe_pFIR_dev_t *dev = (fe_pFIR_dev_t *)platform_get_drvdata(pdev);
  
  pr_info("pFIR_remove enter\n");
  
  cdev_del(&dev->cdev);
  unregister_chrdev_region(dev_num, 2);
  iounmap(dev->regs);
  
  pr_info("pFIR_remove exit\n");
  
  return 0;
}

static void pFIR_exit(void) {
  pr_info("Flat Earth Programmable FIR filter module exit\n");
  platform_driver_unregister(&pFIR_platform);
  pr_info("Flat Earth Programmable FIR filter module successfully unregistered\n");
}

/* End of file */
module_init(pFIR_init);
module_exit(pFIR_exit);
