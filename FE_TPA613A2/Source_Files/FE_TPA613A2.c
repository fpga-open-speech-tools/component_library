/** @file

    This kernel driver controls an HA block on the device

    There are currently two active interfaces to control loading cofficients into the driver.

    1)  The devices will load in /dev as fe_TPA6130A2_NNN and little endian files containing 32bit fixed point values can be passed into this
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
#include <linux/cdev.h>
#include <linux/regmap.h>
#include <linux/i2c.h>


// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tyler Davis <support@flatearthinc.com>");
MODULE_DESCRIPTION("Loadable kernel module for the TPA613A2");
MODULE_VERSION("1.0");

// Index of the first negative value in the look up table below
#define PN_INDEX 54

struct fixed_num
{
    int integer;
    int fraction;
    int fraction_len;
};

// Volume levels defined in Raymond Weber's userspace code (multiplied by ten to elimiate the
// decimal and with the negative values multiplied by negative one)
/** Typedef for a single volume level to hold the db volume level and the matching register value */
typedef struct
{
    uint16_t value;
    uint8_t code;
} volumeLevel;

/** Structure with all possible volume level codes to use in TPA6130_register0.volume 
    See the TPA6130A2 datasheet, pg 17 Table 2 in section 8.4.9 Volume Control.
    The value comes from column 2 in the table.  These volume levels are the absolute 
    value of the table values multipled by 10  The code field is the register control word*/
static volumeLevel VolumeLevels[] =
{
    {.value = 1000,  .code = 0xFF},
    {.value = 595, .code = 0x00},
    {.value = 535, .code = 0x01},
    {.value = 500, .code = 0x02},
    {.value = 475, .code = 0x03},
    {.value = 455, .code = 0x04},
    {.value = 439, .code = 0x05},
    {.value = 414, .code = 0x06},
    {.value = 395, .code = 0x07},
    {.value = 365, .code = 0x08},
    {.value = 353, .code = 0x09},
    {.value = 333, .code = 0x0A},
    {.value = 317, .code = 0x0B},
    {.value = 304, .code = 0x0C},
    {.value = 286, .code = 0x0D},
    {.value = 271, .code = 0x0E},
    {.value = 263, .code = 0x0F},
    {.value = 247, .code = 0x10},
    {.value = 237, .code = 0x11},
    {.value = 225, .code = 0x12},
    {.value = 217, .code = 0x13},
    {.value = 205, .code = 0x14},
    {.value = 196, .code = 0x15},
    {.value = 188, .code = 0x16},
    {.value = 178, .code = 0x17},
    {.value = 170, .code = 0x18},
    {.value = 162, .code = 0x19},
    {.value = 152, .code = 0x1A},
    {.value = 145, .code = 0x1B},
    {.value = 137, .code = 0x1C},
    {.value = 130, .code = 0x1D},
    {.value = 123, .code = 0x1E},
    {.value = 116, .code = 0x1F},
    {.value = 109, .code = 0x20},
    {.value = 103, .code = 0x21},
    {.value = 97,  .code = 0x22},
    {.value = 90,  .code = 0x23},
    {.value = 85,  .code = 0x24},
    {.value = 78,  .code = 0x25},
    {.value = 72,  .code = 0x26},
    {.value = 67,  .code = 0x27},
    {.value = 61,  .code = 0x28},
    {.value = 56,  .code = 0x29},
    {.value = 51,  .code = 0x2A},
    {.value = 45,  .code = 0x2B},
    {.value = 41,  .code = 0x2C},
    {.value = 35,  .code = 0x2D},
    {.value = 31,  .code = 0x1E},
    {.value = 26,  .code = 0x1F},
    {.value = 21,  .code = 0x30},
    {.value = 17,  .code = 0x31},
    {.value = 12,  .code = 0x32},
    {.value = 8,   .code = 0x33},
    {.value = 3,   .code = 0x34},
    {.value = 1,   .code = 0x35},
    {.value = 5,   .code = 0x36},
    {.value = 9,   .code = 0x37},
    {.value = 14,  .code = 0x38},
    {.value = 17,  .code = 0x39},
    {.value = 21,  .code = 0x3A},
    {.value = 25,  .code = 0x3B},
    {.value = 29,  .code = 0x3C},
    {.value = 33,  .code = 0x3D},
    {.value = 36,  .code = 0x3E},
    {.value = 40,  .code = 0x3F}
};

static struct class *cl; // Global variable for the device class
static dev_t dev_num;

// Define some I2C stuff
struct i2c_driver tpa_i2c_driver;
struct i2c_client *tpa_i2c_client;
static const unsigned short normal_i2c[]=
  { 0x35, I2C_CLIENT_END }; // remove?

// Function Prototypes
static int TPA613A2_probe(struct platform_device *pdev);
static int TPA613A2_remove(struct platform_device *pdev);
static ssize_t TPA613A2_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t TPA613A2_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static int TPA613A2_open(struct inode *inode, struct file *file);
static int TPA613A2_release(struct inode *inode, struct file *file);
static ssize_t name_show(struct device *dev, struct device_attribute *attr, char *buf);

// I2C operation prototypes
static ssize_t volume_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t volume_read(struct device *dev, struct device_attribute *attr, char *buf);

// Custom function declarations
char *strcat2(char *dst, char *src);
uint32_t set_fixed_num(const char *s);
int fp_to_string(char *buf, uint32_t fp28_num);
uint8_t find_volume_level(uint32_t fp28_num, uint8_t pn);
uint32_t decode_volume(uint8_t code);

//Create the attributes that show up in /sys/class
static DEVICE_ATTR(volume,          0664, volume_read,          volume_write);

static DEVICE_ATTR(name, 0444, name_show, NULL);

/** An instance of this structure will be created for every fe_HA IP in the system
    This structure holds the linux driver structure as well as a memory pointer to the hardwar and
    shadow registers with the data stored on the device.
*/
struct fe_TPA613A2_dev
{
    struct cdev cdev;           ///< The driver structure containing major/minor, etc
    char *name;                 ///< This gets the name of the device when loading the driver
    void __iomem *regs;         ///< Pointer to the registers on the device
    uint32_t volume;
};


/** Typedef of the driver structure */
typedef struct fe_TPA613A2_dev fe_TPA613A2_dev_t;   //Annoying but makes sonarqube not crash during the analysis in the container_of() lines

/** Id matching structure for use in driver/device matching */
static struct of_device_id fe_TPA613A2_dt_ids[] =
{
    {
        .compatible = "dev,fe-tpa613a2"
    },
    { }
};

static struct i2c_device_id tpa_id[] = {
    {
        "tpa_i2c",0x60
    },
    {}
};

// create the board info and set the register for the ADC (0x60)
static struct i2c_board_info tpa_i2c_info = {
  I2C_BOARD_INFO("tpa_i2c",0x60),
};

static int tpa_i2c_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
  return 0;
}

static int tpa_i2c_remove(struct i2c_client *client)
{
  return 0;
}
struct i2c_driver tpa_i2c_driver = {
    .driver = { 
        .name="tpa_i2c",
      },
    .probe = tpa_i2c_probe,
    .remove = tpa_i2c_remove,
    .id_table = tpa_id,
};


/** Notify the kernel about the driver matching structure information */
MODULE_DEVICE_TABLE(of, fe_TPA613A2_dt_ids);

// Data structure with pointers to the externally important functions to be able to load the module
static struct platform_driver TPA613A2_platform =
{
      .probe = TPA613A2_probe,
      .remove = TPA613A2_remove,
      .driver = {
      .name = "Flat Earth TPA613A2 Driver",
      .owner = THIS_MODULE,
      .of_match_table = fe_TPA613A2_dt_ids
    }
};

/** Structure containing pointers to the functions the driver can load */
static const struct file_operations fe_TPA613A2_fops =
{
    .owner = THIS_MODULE,
    .read = TPA613A2_read,               ///< Read the device contents for the entry in /dev
    .write = TPA613A2_write,             ///< Write the device contents for the entry in /dev
    .open = TPA613A2_open,               ///< Called when the device is opened
    .release = TPA613A2_release,         ///< Called when the device is closes
};



/** Function called initially on the driver loads

    This function is called by the kernel when the driver module is loaded and currently just calls TPA613A2_probe()

    @returns SUCCESS
*/
static int TPA613A2_init(void)
{
    int ret_val = 0;
    struct i2c_adapter *i2c_adapt;
    struct i2c_board_info i2c_info;
    
    pr_info("Initializing the Flat Earth TPA613A2 module\n");

    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&TPA613A2_platform);
    if (ret_val != 0)
    {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }
    
    /*------------------------------------------------------------------
      I2C communication
    ------------------------------------------------------------------*/
    
    // Register the device
    ret_val = i2c_add_driver(&tpa_i2c_driver);
    if (ret_val < 0)
    {
      pr_err("Failed to register I2C driver");
      return ret_val;
    }
    
    i2c_adapt = i2c_get_adapter(0);
    memset(&i2c_info,0,sizeof(struct i2c_board_info));    
    strlcpy(i2c_info.type, "tpa_i2c",I2C_NAME_SIZE);
    
    tpa_i2c_client = i2c_new_device(i2c_adapt,&tpa_i2c_info);
    
    i2c_put_adapter(i2c_adapt);

    if (!tpa_i2c_client)
    {
      pr_err("Failed to connect to I2C client\n");
      ret_val = -ENODEV;
      return ret_val;
    }

    //Send some initialization commands

    // Enable both channels
    char cmd[2] = {0x01, 0xc0};
    i2c_master_send(tpa_i2c_client,&cmd[0],2);

    // Set -.3dB gain on both channels (closest value to unity)
    cmd[0] = 0x02;
    cmd[1] = 0x34;
    i2c_master_send(tpa_i2c_client,&cmd[0],2);

    /*------------------------------------------------------------------
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/
    
    pr_info("Flat Earth TPA6130A2 module successfully initialized!\n");

    return 0;
}



/** Kernel module loading for platform devices

    Called by the kernel when a module is loaded which matches a device tree overlay entry.
    This function does all the setup of the device driver and creates the sysfs entries.

    @param pdev Pointer to a platform_device structure containing information from the overlay about the device to load
    @returns SUCCESS or error code
*/
static int TPA613A2_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;

    char deviceName[20] = "fe_TPA6130A2_";
    char deviceMinor[20];
    int status;

    struct device *deviceObj;
    fe_TPA613A2_dev_t *fe_TPA613A2_devp;

    pr_info("TPA613A2_probe enter\n");

    // Create structure to hold device-specific information (like the registers). Make size of &pdev->dev + sizeof(struct(fe_TPA613A2_dev)).
    fe_TPA613A2_devp = devm_kzalloc(&pdev->dev, sizeof(fe_TPA613A2_dev_t), GFP_KERNEL);

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void *)fe_TPA613A2_devp);

    //Create a memory region to store the device name
    fe_TPA613A2_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
    if (fe_TPA613A2_devp->name == NULL)
        goto bad_mem_alloc;

    //Copy the name from the overlay and stick it in the created memory region
    strcpy(fe_TPA613A2_devp->name, (char *)pdev->name);
    pr_info("%s\n", (char *)pdev->name);

    //Request a Major/Minor number for the driver
    status = alloc_chrdev_region(&dev_num, 0, 1, "fe_TPA6130A2_");
    if (status != 0)
        goto bad_alloc_chrdev_region;

    //Create the device name with the information reserved above
    sprintf(deviceMinor, "%d", MAJOR(dev_num));
    strcat(deviceName, deviceMinor);
    pr_info("%s\n", deviceName);

    //Create sysfs entries
    cl = class_create(THIS_MODULE, deviceName);
    if (cl == NULL)
        goto bad_class_create;

    //Initialize a char dev structure
    cdev_init(&fe_TPA613A2_devp->cdev, &fe_TPA613A2_fops);

    //Registers the char driver with the kernel
    status = cdev_add(&fe_TPA613A2_devp->cdev, dev_num, 1);
    if (status != 0)
        goto bad_cdev_add;

    //Creates the device entries in sysfs
    deviceObj = device_create(cl, NULL, dev_num, NULL, deviceName);
    if (deviceObj == NULL)
        goto bad_device_create;

    //Put a pointer to the fe_fir_dev struct that is created into the driver object so it can be accessed uniquely from elsewhere
    dev_set_drvdata(deviceObj, fe_TPA613A2_devp);

    //---------------------------------------------------------
    status = device_create_file(deviceObj, &dev_attr_volume);
    if (status)
        goto bad_device_create_file_1;

    //---------------------------------------------------------    
    status = device_create_file(deviceObj, &dev_attr_name);
    if (status)
        goto bad_device_create_file_2;

    pr_info("TPA613A2_probe exit\n");

    return 0;

  bad_device_create_file_2:
      device_remove_file(deviceObj, &dev_attr_name);
          
  bad_device_create_file_1:
      device_remove_file(deviceObj, &dev_attr_volume); 
      
  bad_device_create:
      cdev_del(&fe_TPA613A2_devp->cdev);

  bad_cdev_add:
      class_destroy(cl);

  bad_class_create:
      unregister_chrdev_region(dev_num, 1);

  bad_alloc_chrdev_region:
  bad_mem_alloc:

    return ret_val;
}

/** Run when the device opens to create the file structure to read and write

    Beyond creating a structure which the other functions ca use to access the device, this function loads
    the initial values into the shadow registers

    @param inode Pointer to the instance of the hardware driver to use
    @param file Pointer to the file object opened
    @return SUCCESS or error code
*/
static int TPA613A2_open(struct inode *inode, struct file *file)
{
    //Create a pointer to the driver instance
    fe_TPA613A2_dev_t *devp;

    //Put it in the container_of structure so it can be used from anywhere
    devp = container_of(inode->i_cdev, fe_TPA613A2_dev_t, cdev);
    file->private_data = devp;

    //Instantiate default values for the codec
    devp->volume       = 0;

    return 0;
}



/** Called when the device is closed

    Currently this doesn't do anything, but as it is the opposite of open I created it for future use

    @param inode Instance of the driver opened
    @param file Pointer to the file for this operation
    @returns SUCCESS
*/
static int TPA613A2_release(struct inode *inode, struct file *file)
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
    @returns TPA613A2_read Number of bits sent in buffer, and will return 0 for the last transaction.
*/
static ssize_t TPA613A2_read(struct file *file, char *buffer, size_t len, loff_t *offset)
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
    @returns TPA613A2_read Number of bytes written
*/
static ssize_t TPA613A2_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
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
static int TPA613A2_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    fe_TPA613A2_dev_t *dev = (fe_TPA613A2_dev_t *)platform_get_drvdata(pdev);

    pr_info("TPA613A2_remove enter\n");

    // Unregister the character file (remove it from /dev)
    cdev_del(&dev->cdev);

    //Tell the os that the major/minor pair is avalible again
    unregister_chrdev_region(dev_num, 2);

    pr_info("TPA613A2_remove exit\n");

    return 0;
}



// Called when the driver is removed
static void TPA613A2_exit(void)
{
    pr_info("Flat Earth TPA613A2 module exit\n");

    // Unregister our driver from the "Platform Driver" bus
    // This will cause "TPA613A2_remove" to be called for each connected device
    platform_driver_unregister(&TPA613A2_platform);
 
    pr_info("Flat Earth TPA6130A2 module successfully unregistered\n");
}

/** Function to display the overlay name for the device in sysfs

    @todo Better understand the inputs

    @param dev
    @param attr
    @param buf charactor buffer for the sysfs return
    @returns Length of the buffer
*/
static ssize_t name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    fe_TPA613A2_dev_t *devp = (fe_TPA613A2_dev_t *)dev_get_drvdata(dev);

    //Copy the name into the output buffer
    sprintf(buf, "%s\n", devp->name);

    //Return the length of the buffer so it will print in the console
    return strlen(buf);
}

static ssize_t volume_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    // Initialize some variables
    uint32_t tempValue = 0;
    char substring[80];
    int substring_count = 0;
    int i;
    char cmd[2] = {0x02,0x00};
    uint8_t code = 0x00;

    // Create a new instance of the TPA
    fe_TPA613A2_dev_t *devp = (fe_TPA613A2_dev_t *)dev_get_drvdata(dev);

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

      // Find the fp28 number
      tempValue = set_fixed_num(substring);

      // Determine the code for the volume level
      code = find_volume_level(tempValue,0);
    }
    // Otherwise, 
    else
    {
      // Calculate the fp28 number
      tempValue = set_fixed_num(substring);

      // Determine the code for the volume level
      code = find_volume_level(tempValue,1);
    }

    // Determine the closest volume level
    tempValue = decode_volume(code);

    // Record the volume level to the volume variable
    devp->volume = tempValue;

    // Send the I2C commands
    cmd[1] = code;
    i2c_master_send(tpa_i2c_client,&cmd[0],2);

    return count;
}
static ssize_t volume_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    fe_TPA613A2_dev_t *devp = (fe_TPA613A2_dev_t *)dev_get_drvdata(dev);

    fp_to_string(buf, devp->volume);

    strcat2(buf, "\n");

    //Return the length of the buffer so it will print in the console
    return strlen(buf);
}

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
uint8_t find_volume_level(uint32_t fp28_num, uint8_t pn)
{
  // Instantiate some variables
  uint8_t return_code = 0;
  int start_index;
  int stop_index;
  int16_t diff = 10000;
  int16_t curDiff = 10000;
  uint32_t inputnum = fp28_num*10;
  uint16_t intval = (uint16_t)((inputnum)>>=16);
  int i;
  uint16_t return_value = 0;

  // Find the length of the array
  size_t n = sizeof(VolumeLevels)/sizeof(VolumeLevels[0]);
  
  // Set the start and stop search values depending on whether
  // a negative sign was detected
  if (pn == 1)
  {
    start_index = PN_INDEX;
    stop_index = n;
  }
  else
  {  
    start_index = 0;
    stop_index = PN_INDEX - 1;  
  } 
      
  // Determine whether the negative dB bound has been exceeded
  if ((intval > VolumeLevels[0].value) && (pn == 0))
  { 
    printk("Maximum attenuation exceeded.\n");
    printk("Setting attenuation to -100 dB.\n");
    return VolumeLevels[0].code;
  }

  // Determine whether the positive dB bound has been exceeded
  if ((intval > VolumeLevels[n-1].value) && (pn == 1))
  {
    printk("Maximum amplification exceeded.\n");
    printk("Setting amplification to 4 dB.\n");
    return VolumeLevels[n-1].code;
  }  

  // Search for the closest value (above or below)
  for (i = start_index; i < stop_index; i++)
  {
    // calculate the difference values 
    if (intval > VolumeLevels[i].value)
      diff = intval - VolumeLevels[i].value;
    else
      diff = VolumeLevels[i].value - intval;

    // If the calculated difference is smaller than the calculated
    // current best (smallest) difference set the new current difference
    if (diff < curDiff)
    {
      curDiff = diff;
      return_code = VolumeLevels[i].code;
      return_value = VolumeLevels[i].value;
    }
    // Otherwise, stop.  (it only gets worse from here)
    else
      break;
  }
  
  // Return the correct volume code
  return return_code;
}

uint32_t decode_volume(uint8_t code)
{
  // Initialize some variables
  int i;
  size_t n = sizeof(VolumeLevels)/sizeof(VolumeLevels[0]);
  uint32_t decodedDB = 0;

  // Find the correct volume level
  for (i=0;i<n;i++)
  {
    if (code == VolumeLevels[i].code)
      break; 
  }

  // If it's in the negative portion, multiply by -1  
  if (i < PN_INDEX)
  {    
    decodedDB = VolumeLevels[i].value;
    decodedDB = (decodedDB<<=16)/10;
    return -1*decodedDB;
  }  
  else
  {
    decodedDB = VolumeLevels[i].value;
    return (decodedDB<<=16)/10;
  }  


}

/** Tell the kernel what the initialization function is */
module_init(TPA613A2_init);

/** Tell the kernel what the delete function is */
module_exit(TPA613A2_exit);










































