/** @file

    This kernel driver controls an HA block on the device

    There are currently two active interfaces to control loading cofficients into the driver.

    1)  The devices will load in /dev as fe_HANNN and little endian files containing 32bit fixed point values can be passed into this
    to update the cofficient files.  Number of coefficients are automatically computed from the length of this file.  Conversly, this entry can be read to read out the values currently loaded in the the hardware.

    @author Tyler Davis (based on code written by Raymond Weber)
    @copyright 2018 FlatEarth Inc, Bozeman MT
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include<linux/cdev.h>
#include <linux/spi/spi.h>
#include <linux/regmap.h>


// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tyler Davis <support@flatearthinc.com>");
MODULE_DESCRIPTION("Loadable kernel module for the AD7768-4");
MODULE_VERSION("1.0");

static uint8_t bits = 8;
static uint32_t speed = 500000;
static struct spi_device *spi_device;

#define AD7768_WRITE       	0x00
#define AD7768_READ  				0x80

#define CH_STNDBY_ADDR			0x00
#define MODEA_ADDR      		0x01
#define CHMODE_SEL      		0x03
#define POWER_MODE_ADDR			0x04
#define INTFACE_CFG     		0x07
#define ADC0_GAIN_ADDR_MSB  0x36
#define ADC0_GAIN_ADDR_MID  0x37
#define ADC0_GAIN_ADDR_LSB  0x38
#define ADC1_GAIN_ADDR_MSB  0x39
#define ADC1_GAIN_ADDR_MID  0x3A
#define ADC1_GAIN_ADDR_LSB  0x3B


struct fixed_num
{
    int integer;
    int fraction;
    int fraction_len;
};

static struct class *cl; // Global variable for the device class
static dev_t dev_num;

// Function Prototypes
static int AD7768_4_probe(struct platform_device *pdev);
static int AD7768_4_remove(struct platform_device *pdev);
static ssize_t AD7768_4_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t AD7768_4_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static int AD7768_4_open(struct inode *inode, struct file *file);
static int AD7768_4_release(struct inode *inode, struct file *file);
static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf);

// SPI operation prototypes
static ssize_t adc0_gain_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t adc0_gain_read(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t adc1_gain_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t adc1_gain_read(struct device *dev, struct device_attribute *attr, char *buf);

// Custom function declarations
char *strcat2(char *dst, char *src);
uint32_t set_fixed_num(const char *s);
int fp_to_string(char *buf, uint32_t fp28_num);
uint32_t determine_relative_gain(uint32_t fp28_num);

//Create the attributes that show up in /dev/class
static DEVICE_ATTR(adc0_gain,                 0664, adc0_gain_read,               adc0_gain_write);
static DEVICE_ATTR(adc1_gain,                 0664, adc1_gain_read,               adc1_gain_write);

static DEVICE_ATTR(name, 0444, name_read, NULL);

/** An instance of this structure will be created for every fe_HA IP in the system
    This structure holds the linux driver structure as well as a memory pointer to the hardwar and
    shadow registers with the data stored on the device.
*/
struct fe_AD7768_4_dev
{
    struct cdev cdev;           ///< The driver structure containing major/minor, etc
    char *name;                 ///< This gets the name of the device when loading the driver
    void __iomem *regs;         ///< Pointer to the registers on the device
    int adc0_gain;
    int adc1_gain;
};


/** Typedef of the driver structure */
typedef struct fe_AD7768_4_dev fe_AD7768_4_dev_t;   //Annoying but makes sonarqube not crash during the analysis in the container_of() lines

/** Id matching structure for use in driver/device matching */
static struct of_device_id fe_AD7768_4_dt_ids[] =
{
    {
        .compatible = "dev,fe-ad7768_4"
    },
    { }
};

/** Notify the kernel about the driver matching structure information */
MODULE_DEVICE_TABLE(of, fe_AD7768_4_dt_ids);

// Data structure with pointers to the externally important functions to be able to load the module
static struct platform_driver AD7768_4_platform =
{
    .probe = AD7768_4_probe,
    .remove = AD7768_4_remove,
    .driver = {
        .name = "Flat Earth AD7768_4 Driver",
        .owner = THIS_MODULE,
        .of_match_table = fe_AD7768_4_dt_ids
    }
};

/** Structure containing pointers to the functions the driver can load */
static const struct file_operations fe_AD7768_4_fops =
{
    .owner = THIS_MODULE,
    .read = AD7768_4_read,               ///< Read the device contents for the entry in /dev
    .write = AD7768_4_write,             ///< Write the device contents for the entry in /dev
    .open = AD7768_4_open,               ///< Called when the device is opened
    .release = AD7768_4_release,         ///< Called when the device is closes
};

/** Function called initially on the driver loads

    This function is called by the kernel when the driver module is loaded and currently just calls AD7768_4_probe()

    @returns SUCCESS
*/
static int AD7768_4_init(void)
{
    int ret_val = 0;
    char cmd[2] = {0x00,0x00};
    
    // Add the spi master 
    struct spi_master *master;
    
    pr_info("Initializing the Flat Earth AD7768_4 module\n");

    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&AD7768_4_platform);
    if (ret_val != 0)
    {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }
    
    /*------------------------------------------------------------------
    This SPI initialization is based off code written by Piktas Zuikis
    ------------------------------------------------------------------*/
    
    // Register the device
    struct spi_board_info spi_device_info = {
        .modalias = "fe_ad7768_4_",     // Needs to be the same as the device name
        .max_speed_hz = speed,
        .bus_num = 0,             // Determined from /sys/class/spi_master                
        .chip_select = 2,                     // check
        .mode = 0,
    };
    
    /*To send data we have to know what spi port/pins should be used. This information 
      can be found in the device-tree. */
    master = spi_busnum_to_master( spi_device_info.bus_num );
    if( !master ){
        printk("MASTER not found.\n");
            return -ENODEV;
    }
     
    // create a new slave device, given the master and device info
    spi_device = spi_new_device( master, &spi_device_info );
 
    printk("Setting up new slave device\n");
    if( !spi_device ) {
        printk("FAILED to create slave.\n");
        return -ENODEV;
    }
     
    printk("Set the bits per word\n");
    spi_device->bits_per_word = bits;
 
    printk("Setting up the device\n");
    ret_val = spi_setup( spi_device );
    //printk("%d\n",ret_val);
    if( ret_val ){
        printk("FAILED to setup slave.\n");
        spi_unregister_device( spi_device );
        return -ENODEV;
    }  

    printk("Sending SPI initialization commands...\n");

    // Set the channel standby
    printk("\tSetting standby parameters\n");
    cmd[0] = AD7768_WRITE | CH_STNDBY_ADDR;
    cmd[1] = 0xFC;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));

    // Set the channel mode parameters
    printk("\tSetting Mode A parameters\n");
    cmd[0] = AD7768_WRITE | MODEA_ADDR;
    cmd[1] = 0x09;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    //printk("%d\n",ret_val);

    // Set the power mode
    printk("\tSetting power parameters\n");
    cmd[0] = AD7768_WRITE | POWER_MODE_ADDR;
    cmd[1] = 0x30;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    //printk("%d\n",ret_val);

//    // Set the mode for the different channels
//    printk("\tSetting channel mode\n");
//    cmd[0] =  write | CHMODE_SEL;
//    cmd[1] = 0x00;
//    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    //printk("%d\n",ret_val);			

    // Set the interface configuration (clock divider)
    printk("\tSetting channel interface configuration\n");
    cmd[0] = AD7768_WRITE | INTFACE_CFG;
    cmd[1] = 0x00;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    //printk("%d\n",ret_val); 

    // Set the channel 0 gain.  (0x555555 is factory default)
    printk("\tSetting channel 0 gain\n");
    cmd[0]  = AD7768_WRITE | ADC0_GAIN_ADDR_MSB;
    cmd[1]  =  0x55;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0]  = AD7768_WRITE | ADC0_GAIN_ADDR_MID;
    cmd[1]  = 0x55;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0]  = AD7768_WRITE | ADC0_GAIN_ADDR_LSB;
    cmd[1]  = 0x55;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    //printk("%d\n",ret_val); 

    // Set the channel 1 gain.  (0x555555 is factory default)
    printk("\tSetting channel 1 gain\n");
    cmd[0] = AD7768_WRITE | ADC1_GAIN_ADDR_MSB;
    cmd[1] = 0x55;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0] = AD7768_WRITE | ADC1_GAIN_ADDR_MID;
    cmd[1] = 0x55;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0] = AD7768_WRITE | ADC1_GAIN_ADDR_LSB;
    cmd[1] = 0x55;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    //printk("%d\n",ret_val); 
    

    /*------------------------------------------------------------------
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/
    
    pr_info("Flat Earth AD7768-4 module successfully initialized!\n");

    return 0;
}



/** Kernel module loading for platform devices

    Called by the kernel when a module is loaded which matches a device tree overlay entry.
    This function does all the setup of the device driver and creates the sysfs entries.

    @param pdev Pointer to a platform_device structure containing information from the overlay about the device to load
    @returns SUCCESS or error code
*/
static int AD7768_4_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;

    char deviceName[20] = "fe_AD7768_4_";
    char deviceMinor[20];
    int status;

    struct device *deviceObj;
    fe_AD7768_4_dev_t *fe_AD7768_4_devp;

    pr_info("AD7768_4_probe enter\n");

    // Create structure to hold device-specific information (like the registers). Make size of &pdev->dev + sizeof(struct(fe_AD7768_4_dev)).
    fe_AD7768_4_devp = devm_kzalloc(&pdev->dev, sizeof(fe_AD7768_4_dev_t), GFP_KERNEL);

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void *)fe_AD7768_4_devp);

    //Create a memory region to store the device name
    fe_AD7768_4_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
    if (fe_AD7768_4_devp->name == NULL)
        goto bad_mem_alloc;

    //Copy the name from the overlay and stick it in the created memory region
    strcpy(fe_AD7768_4_devp->name, (char *)pdev->name);
    pr_info("%s\n", (char *)pdev->name);

    //Request a Major/Minor number for the driver
    status = alloc_chrdev_region(&dev_num, 0, 1, "fe_HA");
    if (status != 0)
        goto bad_alloc_chrdev_region;

    //Create the device name with the information reserved above
    sprintf(deviceMinor, "%d", MAJOR(dev_num));
    strcat(deviceName, deviceMinor);
    pr_info("%s\n", deviceName);

    //Create sysfs entries
    cl = class_create(THIS_MODULE, deviceName); //"fe_HA");
    if (cl == NULL)
        goto bad_class_create;

    //Initialize a char dev structure
    cdev_init(&fe_AD7768_4_devp->cdev, &fe_AD7768_4_fops);

    //Registers the char driver with the kernel
    status = cdev_add(&fe_AD7768_4_devp->cdev, dev_num, 1);
    if (status != 0)
        goto bad_cdev_add;

    //Creates the device entries in sysfs
    deviceObj = device_create(cl, NULL, dev_num, NULL, deviceName);
    if (deviceObj == NULL)
        goto bad_device_create;

    //Put a pointer to the fe_fir_dev struct that is created into the driver object so it can be accessed uniquely from elsewhere
    dev_set_drvdata(deviceObj, fe_AD7768_4_devp);

    //---------------------------------------------------------

    status = device_create_file(deviceObj, &dev_attr_adc0_gain);
    if (status)
        goto bad_device_create_file_1;

    //---------------------------------------------------------

    status = device_create_file(deviceObj, &dev_attr_adc1_gain);
    if (status)
        goto bad_device_create_file_2;

    //---------------------------------------------------------    
    status = device_create_file(deviceObj, &dev_attr_name);
    if (status)
        goto bad_device_create_file_3;

    pr_info("AD7768_4_probe exit\n");

    return 0;

bad_device_create_file_3:
    device_remove_file(deviceObj, &dev_attr_name);
    
bad_device_create_file_2:
    device_remove_file(deviceObj, &dev_attr_adc1_gain);
        
bad_device_create_file_1:
    device_remove_file(deviceObj, &dev_attr_adc0_gain);
        
bad_device_create:
    cdev_del(&fe_AD7768_4_devp->cdev);

bad_cdev_add:
    class_destroy(cl);

bad_class_create:
    unregister_chrdev_region(dev_num, 1);

bad_alloc_chrdev_region:
bad_mem_alloc:

//bad_ioremap:
//    ret_val = PTR_ERR(fe_AD7768_4_devp->regs);

    return ret_val;
}

/** Run when the device opens to create the file structure to read and write

    Beyond creating a structure which the other functions ca use to access the device, this function loads
    the initial values into the shadow registers

    @param inode Pointer to the instance of the hardware driver to use
    @param file Pointer to the file object opened
    @return SUCCESS or error code
*/
static int AD7768_4_open(struct inode *inode, struct file *file)
{
    //Create a pointer to the driver instance
    fe_AD7768_4_dev_t *devp;

    //Put it in the container_of structure so it can be used from anywhere
    devp = container_of(inode->i_cdev, fe_AD7768_4_dev_t, cdev);
    file->private_data = devp;

    //Instantiate default values for the ADC

    devp->adc0_gain       = 0.0;
    devp->adc1_gain       = 0.0;
    return 0;
}



/** Called when the device is closed

    Currently this doesn't do anything, but as it is the opposite of open I created it for future use

    @param inode Instance of the driver opened
    @param file Pointer to the file for this operation
    @returns SUCCESS
*/
static int AD7768_4_release(struct inode *inode, struct file *file)
{
    return 0;
}



/** Read the contents of the coefficients stucture

    This function will read the contents of the coefficient memory as stored in the shadow register and return then
    to the calling function as a binary array of 32 length fixed point values.  If done in the terminal window, hexdump
    can be used to see the values.

    @param file Pointer to the file being accessed
    @param buffer Pointer to a buffer array to return the data on
    @len Unused
    @offset Pass-by-reference variable to hold where to start transmitting from in the array.
    @returns AD7768_4_read Number of bits sent in buffer, and will return 0 for the last transaction.
*/
static ssize_t AD7768_4_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    return 0;
}



/** Write the contents of the coefficients stucture

    This function will write the contents of the buffer as binary values to the coefficients register.
    The length variable is used to set the length of the filter itself.  After updating the shadow register,
    the values are then written to the device.  If the size of buffer is not a 4 bit multiple, the extra bytes
    are ignored.

    @param file Pointer to the file being written to
    @param buffer Pointer to a buffer array containing the data to write
    @len Number of bytes in the buffer variable
    @offset Pass-by-reference variable to hold where to start transmitting from in the array.
    @returns AD7768_4_read Number of bytes written
*/
static ssize_t AD7768_4_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    return 0;
}



/** Function called when the platform device driver is deleted

    This function is called when the device driver is deleted.  It should cleans up the driver memory structures,
    deallocate the driver addresses reserved for the driver, unallocate memory and the io mapping functions to the
    hardware.  After this function, the device should be able to be added cleanly again without contention or memory
    leaks.

    @param platform_device Pointer to the device structure being deleted
    @returns SUCCESS
*/
static int AD7768_4_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    fe_AD7768_4_dev_t *dev = (fe_AD7768_4_dev_t *)platform_get_drvdata(pdev);

    //device_remove_file(deviceObj, &dev_attr_numCoefs);
    //device_remove_file(deviceObj, &dev_attr_name);
    //device_remove_file(deviceObj, &dev_attr_name);

    //device_destroy(cl, dev_num);
    //class_destroy(cl);


    pr_info("AD7768_4_remove enter\n");

    // Turn the HA off
    //iowrite32(0x00, dev->regs);

    // Unregister the character file (remove it from /dev)
    cdev_del(&dev->cdev);

    //Tell the os that the major/minor pair is avalible again
    unregister_chrdev_region(dev_num, 2);

    //Remove the pointer to the registers so it doesn't leak
    //iounmap(dev->regs);

    pr_info("AD7768_4_remove exit\n");

    return 0;
}



// Called when the driver is removed
static void AD7768_4_exit(void)
{
    pr_info("Flat Earth AD7768_4 module exit\n");

    // Unregister our driver from the "Platform Driver" bus
    // This will cause "AD7768_4_remove" to be called for each connected device
    platform_driver_unregister(&AD7768_4_platform);
 
    if( spi_device ){
        spi_unregister_device( spi_device );
    }

    pr_info("Flat Earth AD7768_4 module successfully unregistered\n");
}

/** Function to display the overlay name for the device in sysfs

    @todo Better understand the inputs

    @param dev
    @param attr
    @param buf charactor buffer for the sysfs return
    @returns Length of the buffer
*/
static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    fe_AD7768_4_dev_t *devp = (fe_AD7768_4_dev_t *)dev_get_drvdata(dev);

    //Copy the name into the output buffer
    sprintf(buf, "%s\n", devp->name);

    //Return the length of the buffer so it will print in the console
    return strlen(buf);
}

static ssize_t adc0_gain_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    uint32_t tempValue = 0;
    uint32_t volume_level;
    char cmd[2] = {0x00,0x00};

    char substring[80];
    int substring_count = 0;
    int i;
    int ret_val;

    fe_AD7768_4_dev_t *devp = (fe_AD7768_4_dev_t *)dev_get_drvdata(dev);

    for (i = 0; i < count; i++)
    {
        //If its not a space or a comma, add the digit to the substring
        if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n'))
        {
            substring[substring_count] = buf[i];
            substring_count++;
        }
    }

    substring[substring_count] = '\0';

    // If the first character is a negative sign
    if (buf[0] == '-')
    {
      // Shift all the values "left" to remove it
      for (i=0;i<79;i++)
        substring[i] = substring[i+1];
    }

    //Convert the buffer to a fixed point value
    tempValue = set_fixed_num(substring);

    //printk("Entering conversion function\n");
    volume_level = determine_relative_gain(tempValue);
    
    //Write the value into the shadow register (and pray it works...)
    devp->adc0_gain = tempValue;

    printk("\tSetting channel 0 gain\n");
    cmd[0]  = AD7768_WRITE | ADC0_GAIN_ADDR_MSB;
    cmd[1]  =  (uint8_t)(volume_level>>16);
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0]  = AD7768_WRITE | ADC0_GAIN_ADDR_MID;
    cmd[1]  = (uint8_t)(volume_level>>8);
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0]  = AD7768_WRITE | ADC0_GAIN_ADDR_LSB;
    cmd[1]  = (uint8_t)volume_level;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));

 //   // Write the SPI commands to the DAC
 //  cmd[2] = volume_level;
 //   spi_write(spi_device,&cmd, sizeof(cmd));

    return count;
}
static ssize_t adc0_gain_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    fe_AD7768_4_dev_t *devp = (fe_AD7768_4_dev_t *)dev_get_drvdata(dev);

    //Copy the name into the output buffer
    //sprintf(buf, "%d\n", fe_HA_devp->threshold);
    fp_to_string(buf, devp->adc0_gain);

    strcat2(buf, "\n");

    //Return the length of the buffer so it will print in the console
    return strlen(buf);
}

static ssize_t adc1_gain_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    uint32_t tempValue = 0;
    uint32_t volume_level;
    char cmd[2] = {0x00,0x00};
    
    char substring[80];
    int substring_count = 0;
    int i;
    int ret_val;

    fe_AD7768_4_dev_t *devp = (fe_AD7768_4_dev_t *)dev_get_drvdata(dev);

    for (i = 0; i < count; i++)
    {
        //If its not a space or a comma, add the digit to the substring
        if ((buf[i] != ',') && (buf[i] != ' ') && (buf[i] != '\0') && (buf[i] != '\r') && (buf[i] != '\n'))
        {
            substring[substring_count] = buf[i];
            substring_count++;
        }
    }

    substring[substring_count] = '\0';

    // If the first character is a negative sign
    if (buf[0] == '-')
    {
      // Shift all the values "left" to remove it
      for (i=0;i<79;i++)
        substring[i] = substring[i+1];
    }

    //Convert the buffer to a fixed point value
    tempValue = set_fixed_num(substring);

    //printk("Entering conversion function\n");
    volume_level = determine_relative_gain(tempValue);
    
    //Write the value into the shadow register (and pray it works...)
    devp->adc1_gain = tempValue;

    printk("\tSetting channel 0 gain\n");
    cmd[0]  = AD7768_WRITE | ADC1_GAIN_ADDR_MSB;
    cmd[1]  =  (uint8_t)(volume_level>>16);
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0]  = AD7768_WRITE | ADC1_GAIN_ADDR_MID;
    cmd[1]  = (uint8_t)(volume_level>>8);
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));
    cmd[0]  = AD7768_WRITE | ADC1_GAIN_ADDR_LSB;
    cmd[1]  = (uint8_t)volume_level;
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));

    return count;
}
static ssize_t adc1_gain_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    fe_AD7768_4_dev_t *devp = (fe_AD7768_4_dev_t *)dev_get_drvdata(dev);

    //Copy the name into the output buffer
    //sprintf(buf, "%d\n", fe_HA_devp->threshold);
    fp_to_string(buf, devp->adc1_gain);

    strcat2(buf, "\n");

    //Return the length of the buffer so it will print in the console
    return strlen(buf);
}

//---------------------------------------------------------------

char *strcat2(char *dst, char *src)
{
    char *cp = dst;

    while (*cp)
        cp++; /* find end of dst */

    while (( *cp++ = *src++ ) != 0); /* Copy src to end of dst */

    return dst; /* return dst */
}

/** Convert a string to a fixed point number structure
    @param s String containing the string to convert to 32F16 representation
    @return SUCCESS

    @todo This function is really ugly and could be cleaned up to make it clear what is happening....
*/
uint32_t set_fixed_num(const char *s)
{
    struct fixed_num num = {0, 0, 0};
    int seen_point = 0;
    int pointIndex;
    int i;
    int ii;
    int frac_comp;
    uint32_t acc = 0;
    char s2[80];
    int pointsSeen = 0;
    int charIndex = 0;

    //If no leading 0, add one (eg: .25 -> 0.25)
    if (s[0] == '.')
    {
        s2[0] = '0';
        charIndex++;
    }

    //This is a strcpy() to move the data a "const char *" to a "char *" and validate the data
    for (i = 0; i < strlen(s); i++)
    {
        //Make sure the string contains an non-valid char (eg: not a number or a decimal point)
        if ((s[i] == '.') || (s[i] >= '0' && s[i] <= '9'))
        {
            //Copy the data over and increment the pointer
            s2[charIndex] = s[i];
            charIndex++;
        }
        else
        {
            pr_info("Invalid char (c:%c x:%X) in number %s\n", s[i], s[i], s);
            return 0x00000000;
        }

        //Count the number of decimals in the string
        if (s[i] == '.')
            pointsSeen++;
    }

    //If multiple decimals points in the number (eg: 1.1.4)
    if (pointsSeen > 1)
        pr_info("Invalid number format: %s\n", s);

    //Turn 1 into 1.0
    //if (pointsSeen == 0)
    // {
    //    printk("Adding the decimal...\n");
    //    s2[i] = char(".");
    //    s2[i+1] = char("0");
    //    //strcat2(".0",s2);
    //    i=i+2;
    // }
    //Make sure the string is terminated
    s2[i] = '\0';

    //Count the fractional digits
    for (pointIndex = 0; pointIndex < strlen(s2); pointIndex++)
    {
        if (s2[pointIndex] == '.')
            break;
    }

    //String extend so that the output is accurate
    while (strlen(s2) - pointIndex < 9)
        strcat2(s2, "0");

    //Truncate the string if its longer
    s2[strlen(s2) - pointIndex + 9] = '\0';

    //Covert to fixed point
    for (i = 0; i < 10; i++)
    {
        if (s2[i] == '.')
        {
            seen_point = 1;
            continue;
        }
        if (!seen_point)
        {
            num.integer *= 10;
            num.integer += (int)(s2[i] - '0');
        }
        else
        {
            num.fraction_len++;
            num.fraction *= 10;
            num.fraction += (int)(s2[i] - '0');
        }
    }

    //Turn the fixed point conversion into binary digits
    for (ii = 0, frac_comp = 1; ii < num.fraction_len; ii++) frac_comp *= 10;
    frac_comp /= 2;

    // Get the fractional part (f28 hopefully)
    for (ii = 0; i <= 36; i++)
    {
        if (num.fraction >= frac_comp)
        {
            acc |= 0x00000001;
            num.fraction -= frac_comp;
        }
        frac_comp /= 2;

        acc = acc << 1;
    }

    acc = acc >> 12;

    //Combine the fractional part with the integer
    acc += num.integer << 16;

    return acc;
}

/** Function to convert a fp16 to a string representation
    @todo doesn't handle negative numbers
*/
int fp_to_string(char *buf, uint32_t fp28_num)
{
    int buf_pos = 0;
    int i;
    int fractionPart;
    //int isNegative = 0;
    int intPart = 1;
    int i16 = 0;

    if (fp28_num & 0x80000000)
    {
        fp28_num *= -1;

        buf[buf_pos] = '-';
        buf_pos++;
    }

    //Convert the integer part
    i16 = (fp28_num >> 16);
    while ( (i16 / intPart) > 9)
    {
        intPart *= 10;
    }

    while (intPart > 0)
    {
        buf[buf_pos] = (char)((i16 / intPart) + '0');
        buf_pos++;

        i16 = i16 % intPart;
        intPart = intPart / 10;
    }

    //buf[buf_pos] = (char)((fp28_num>>16) + '0');
    //buf_pos++;

    buf[buf_pos] = '.';
    buf_pos++;

    //Mask the integer bits and dump 1 bit to make the conversion easier....
    fractionPart = (0x0000FFFF & fp28_num) >> 1; // 32F27 so that 0-9 can fit in the high 5 bits)

    for (i = 0; i < 8; i++)
    {
        fractionPart *= 10;
        buf[buf_pos] = (fractionPart >> 15) + '0';
        buf_pos++;
        fractionPart &= 0x00007FFF;
    }

    buf[buf_pos] = '\0';

    return 0;
}

/** Converts a 32 bit integer into an 8 bit volume level
    @param 32f16representation of a fixed point number
    @return volume_level an 8 bit representation of the attenuation
*/
uint32_t determine_relative_gain(uint32_t fp32_num)
{
  // Initialize the volume level
  uint32_t relative_gain_level = 0;

  // slope and intercept for when 1 <= G_rel < 4;
  uint32_t upper_m = 0x0038E38E;
  uint32_t upper_b = 0x001C71C7;

  // slope for when 0 <= G_rel < 1 (no intercept)
  uint32_t lower_m = 0x00555555;

  // Shift the data to match the representation 
  uint16_t dec_val = fp32_num>>16;
  uint32_t inputnum = (fp32_num*10)>>16;

  printk("inputnum: %d, inputnum/10: %d, fp32_num: %d\n",inputnum,inputnum/10,fp32_num);
  
  // If the gain is over the maximum, set the return to the maximum value
  if (dec_val > 4)
  {
  	relative_gain_level = 0x00FFFFFF;
  }
  // If the gain value is less than "1", use the equation with no intercept
  // to calculate the relative gain value
  else if (dec_val < 1)
  {
  	printk("dec_val < 1\n");
  	relative_gain_level = lower_m * inputnum / 10 ;
  }
  else
  {
  	printk("1 < dec_val < 4\n");
  	relative_gain_level = upper_m * inputnum / 10 + upper_b;
  	printk("m term: %x, +b term: %x\n",upper_m*inputnum/10,upper_m * inputnum / 10 + upper_b);
  }

  printk("dec_val: %d\n");
  printk("The new gain is %x\n",relative_gain_level);

  return relative_gain_level;
}

/** Tell the kernel what the initialization function is */
module_init(AD7768_4_init);

/** Tell the kernel what the delete function is */
module_exit(AD7768_4_exit);

