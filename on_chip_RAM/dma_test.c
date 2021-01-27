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

#include <asm/uaccess.h>  /* for put_user */
#include <asm/errno.h>
#include "custom_functions.h"

static struct class *cl;
static dev_t dev_num;

/* Device struct */
struct fe_dma_dev {
  struct cdev cdev;
  void __iomem *regs;
  char *name;
  unsigned int address;
  unsigned int memory;
};


#define SUCCESS 0
#define DEVICE_NAME "chardev" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80            /* Max length of the message from the device */

static int Device_Open = 0;  /* Is device open?  Used to prevent multiple
                                        access to the device */
static char msg[BUF_LEN];    /* The msg the device will give when asked    */
static char *msg_Ptr;

static int dma_init(void);
static void dma_exit(void);
static int dma_probe(struct platform_device *pdev);
static int dma_remove(struct platform_device *pdev);
static ssize_t dma_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t dma_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static int dma_open(struct inode *inode, struct file *file);
static int dma_release(struct inode *inode, struct file *file);

static ssize_t name_read  (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t address_write (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t address_read  (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t memory_write (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t memory_read  (struct device *dev, struct device_attribute *attr, char *buf);
typedef struct fe_dma_dev fe_dma_dev_t;
/* ID Matching struct */
static struct of_device_id fe_dma_dt_ids[] = {
  {
    .compatible = "dev,al-dma-test"
  },
  { }
};

/* Platform driver struct */
static struct platform_driver dma_platform = {
  .probe = dma_probe,
  .remove = dma_remove,
  .driver = {
    .name = "Flat Earth DMA Driver",
    .owner = THIS_MODULE,
    .of_match_table = fe_dma_dt_ids
  }
};

/* File ops struct */
static const struct file_operations fe_dma_fops = {
  .owner = THIS_MODULE,
  .read = dma_read,
  .write = dma_write,
  .open = dma_open,
  .release = dma_release,
};


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autogen <support@flatearthinc.com");
MODULE_DESCRIPTION("Loadable kernel module for the dma");
MODULE_VERSION("1.0");
MODULE_DEVICE_TABLE(of, fe_dma_dt_ids);
module_init(dma_init);
module_exit(dma_exit);

DEVICE_ATTR(name, 0444, name_read, NULL);
DEVICE_ATTR(address, 0664, address_read, address_write);
DEVICE_ATTR(memory, 0664, memory_read, memory_write);
static struct attribute *dma_attrs[] = {  &dev_attr_name.attr,  &dev_attr_address.attr,  &dev_attr_memory.attr,  NULL};

ATTRIBUTE_GROUPS(dma);

static int dma_init(void) {
  int ret_val = 0;
  //printk(KERN_ALERT "FUNCTION AUTO GENERATED AT: 2020-10-13 12:39\n");
  pr_info("Initializing the Flat Earth dma module\n");
  // Register our driver with the "Platform Driver" bus
  ret_val = platform_driver_register(&dma_platform);  if (ret_val != 0) {
    pr_err("platform_driver_register returned %d\n", ret_val);
    return ret_val;
  }
  pr_info("Flat Earth dma module successfully initialized!\n");
  return 0;
}
static int dma_probe(struct platform_device *pdev) {
  int ret_val = -EBUSY;
  char device_name[29] = "fe_dma";
  char deviceMinor[20];
  int status;
  struct device *device_obj;
  fe_dma_dev_t * fe_dma_devp;
  struct resource *r = NULL;
  pr_info("dma_probe enter\n");
  fe_dma_devp = devm_kzalloc(&pdev->dev, sizeof(fe_dma_dev_t), GFP_KERNEL);
  r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (r == NULL) {
    pr_err("IORESOURCE_MEM (register space) does not exist\n");
    goto bad_exit_return;  }
  fe_dma_devp->regs = devm_ioremap_resource(&pdev->dev, r);
  if (IS_ERR(fe_dma_devp->regs)) {
    ret_val = PTR_ERR(fe_dma_devp->regs);
    goto bad_exit_return;
  }
  platform_set_drvdata(pdev, (void *)fe_dma_devp);
  fe_dma_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
  if (fe_dma_devp->name == NULL)
    goto bad_mem_alloc;
  strcpy(fe_dma_devp->name, (char *)pdev->name);
  pr_info("%s\n", (char *)pdev->name);
  status = alloc_chrdev_region(&dev_num, 0, 1, "fe_dma_");
  if (status != 0)
    goto bad_alloc_chrdev_region;
  
  cl = class_create(THIS_MODULE, device_name);
  if (cl == NULL)
    goto bad_class_create;
  cdev_init(&fe_dma_devp->cdev, &fe_dma_fops);
  status = cdev_add(&fe_dma_devp->cdev, dev_num, 1);
  if (status != 0)
    goto bad_cdev_add;
 
 sprintf(deviceMinor, "%d", MINOR(dev_num));
 strcat(device_name, deviceMinor);
 pr_info("%s\n", device_name);
 device_obj = device_create_with_groups(cl, NULL, dev_num, NULL, dma_groups, device_name);
  if (device_obj == NULL)
    goto bad_device_create;
  dev_set_drvdata(device_obj, fe_dma_devp);
  pr_info("dma exit\n");
  return 0;
bad_device_create:
bad_cdev_add:
  cdev_del(&fe_dma_devp->cdev);
bad_class_create:
  class_destroy(cl);
bad_alloc_chrdev_region:
  unregister_chrdev_region(dev_num, 1);
bad_mem_alloc:
bad_exit_return:
  pr_info("dma_probe bad exit\n");
  return ret_val;
}


static int dma_open(struct inode *inode, struct file *file) {
  static int counter = 0;
   if (Device_Open) return -EBUSY;
  
   Device_Open++;
   sprintf(msg,"I already told you %d times Hello world!\n", counter++);
   msg_Ptr = msg;
   //MOD_INC_USE_COUNT;

   return SUCCESS;
}

static int dma_release(struct inode *inode, struct file *file) {
  Device_Open --;     /* We're now ready for our next caller */

   /* Decrement the usage count, or else once you opened the file, you'll
                    never get get rid of the module. */
  //MOD_DEC_USE_COUNT;

  return 0;
}

static ssize_t dma_read(struct file *file, char *buffer, size_t len, loff_t *offset) {
   /* Number of bytes actually written to the buffer */
   int bytes_read = 0;
   char temp[80];
   int count = 0, n;
   /* If we're at the end of the message, return 0 signifying end of file */
   if (*msg_Ptr == 0) return 0;
  
    
   n = sprintf(temp,"%s", msg_Ptr);
   

   /* Actually put the data into the buffer */
   while (len && *msg_Ptr)  {

        if(len < 5 && count < 150){
          strcat(msg_Ptr, temp);
          len = len + n;
          count ++;
          printk("Made it in");
        }
        /* The buffer is in the user data segment, not the kernel segment;
         * assignment won't work.  We have to use put_user which copies data from
         * the kernel data segment to the user data segment. */
         put_user(*(msg_Ptr++), buffer++);

         len--;
         bytes_read++;
   }

   /* Most read functions return the number of bytes put into the buffer */
   return bytes_read;
}

static ssize_t dma_write(struct file *file, const char *buffer, size_t len, loff_t *offset) {
  printk ("<1>Sorry, this operation isn't supported.\n");
   return -EINVAL;
}
static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_dma_dev_t * devp = (fe_dma_dev_t *)dev_get_drvdata(dev);
  sprintf(buf, "%s\n", devp->name);
  return strlen(buf);
}

static ssize_t address_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_dma_dev_t * devp = (fe_dma_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->address, 0, false, 1);
  sprintf(buf, "%u\n", devp->address);
  return strlen(buf);
}

static ssize_t address_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  unsigned int tempValue = 0;
  fe_dma_dev_t *devp = (fe_dma_dev_t *)dev_get_drvdata(dev);
  
  sscanf(buf, "%u", &tempValue);
  devp->address = tempValue;
  return count;
}

static ssize_t memory_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_dma_dev_t * devp = (fe_dma_dev_t *)dev_get_drvdata(dev);
  char temp[4];
  //int i; 
  sprintf(buf," ");
  
  //for(i = 0; i < 256; i++){
    devp->memory = ioread32((u32 *)devp->regs + devp->address);
    sprintf(temp, "%u", devp->memory);
    strcat(buf, temp);
  //  strcat(buf, " ");
  //}
  //sprintf(buf, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u", devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory, devp->memory);
  //devp->address = devp->address + 1;
  return strlen(buf);
}

static ssize_t memory_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
  unsigned int data, addr;
  fe_dma_dev_t *devp = (fe_dma_dev_t *)dev_get_drvdata(dev);
  sscanf(buf, "%u %u", &addr, &data);
  iowrite32(data, (u32 *)devp->regs + addr);
  
  devp->address = addr;
  return count;
}

static int dma_remove(struct platform_device *pdev) {
  fe_dma_dev_t *dev = (fe_dma_dev_t *)platform_get_drvdata(pdev);
  pr_info("dma_remove enter\n");
  device_destroy(cl, dev_num);
  cdev_del(&dev->cdev);
  class_destroy(cl);
  unregister_chrdev_region(dev_num, 2);
  pr_info("dma_remove exit\n");
  return 0;
}


static void dma_exit(void) {
  pr_info("Flat Earth dma module exit\n");
  platform_driver_unregister(&dma_platform);
  pr_info("Flat Earth dma module successfully unregistered\n");
}
