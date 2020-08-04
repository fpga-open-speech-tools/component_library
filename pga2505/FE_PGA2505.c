/** @file

  This kernel driver controls the gain of the PGA2505 microphone preamp.

  @copyright Copyright 2020 Audio Logic Inc

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Tyler Davis
  Audio Logic Inc
  985 Technology Blvd
  Bozeman, MT 59718
  openspeech@flatearthinc.com
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
#include <linux/spi/spi.h>

// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tyler Davis <openspeech@flatearthinc.com>");
MODULE_DESCRIPTION("Loadable kernel module for the PGA2505");
MODULE_VERSION("1.0");

// Define the number of amplifiers on the hardware
#define NAMP 2

// Define the number of bytes to make the command
#define NCMD 2

struct fixed_num
{
    int integer;
    int fraction;
    int fraction_len;
};

static uint8_t bits = 16;
static uint32_t speed = 500000;
static struct spi_device *spi_device;

static struct class *cl; // Global variable for the device class
static dev_t dev_num;

// Function Prototypes
static int PGA2505_probe(struct platform_device *pdev);
static int PGA2505_remove(struct platform_device *pdev);
static ssize_t PGA2505_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t PGA2505_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static int PGA2505_open(struct inode *inode, struct file *file);
static int PGA2505_release(struct inode *inode, struct file *file);
static ssize_t name_show(struct device *dev, struct device_attribute *attr, char *buf);

// SPI operation prototypes
static ssize_t volume_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t volume_read(struct device *dev, struct device_attribute *attr, char *buf);

// Custom function declarations
char *strcat2(char *dst, char *src);
uint32_t set_fixed_num(const char *s);
int fp_to_string(char *buf, uint32_t fp28_num);
uint8_t find_volume_level(uint32_t fp28_num);
uint32_t decode_volume(uint8_t code);
uint8_t encode_gpio(uint8_t code);
void fill_cmd_array(char *cmd, uint8_t code, uint8_t defreg);

//Create the attributes that show up in /sys/class
static DEVICE_ATTR(volume,          0664, volume_read,          volume_write);

static DEVICE_ATTR(name, 0444, name_show, NULL);

/** An instance of this structure will be created for every fe_HA IP in the system
    This structure holds the linux driver structure as well as a memory pointer to the hardwar and
    shadow registers with the data stored on the device.
*/
struct fe_PGA2505_dev
{
    struct cdev cdev;           ///< The driver structure containing major/minor, etc
    char *name;                 ///< This gets the name of the device when loading the driver
    void __iomem *regs;         ///< Pointer to the registers on the device
    uint32_t volume;
};


/** Typedef of the driver structure */
typedef struct fe_PGA2505_dev fe_PGA2505_dev_t;   //Annoying but makes sonarqube not crash during the analysis in the container_of() lines

/** Id matching structure for use in driver/device matching */
static struct of_device_id fe_PGA2505_dt_ids[] =
{
    {
        .compatible = "dev,fe-pga2505"
    },
    { }
};


/** Notify the kernel about the driver matching structure information */
MODULE_DEVICE_TABLE(of, fe_PGA2505_dt_ids);

// Data structure with pointers to the externally important functions to be able to load the module
static struct platform_driver PGA2505_platform =
{
      .probe = PGA2505_probe,
      .remove = PGA2505_remove,
      .driver = {
      .name = "Audio Logic PGA2505 Driver",
      .owner = THIS_MODULE,
      .of_match_table = fe_PGA2505_dt_ids
    }
};

/** Structure containing pointers to the functions the driver can load */
static const struct file_operations fe_PGA2505_fops =
{
    .owner = THIS_MODULE,
    .read = PGA2505_read,               ///< Read the device contents for the entry in /dev
    .write = PGA2505_write,             ///< Write the device contents for the entry in /dev
    .open = PGA2505_open,               ///< Called when the device is opened
    .release = PGA2505_release,         ///< Called when the device is closes
};



/** Function called initially on the driver loads

    This function is called by the kernel when the driver module is loaded and currently just calls PGA2505_probe()

    @returns SUCCESS
*/
static int PGA2505_init(void)
{
    int ret_val = 0;
    int i = 0;

    // Add the spi master
    struct spi_master *master;

    // Initialize the command array, disable all servos, zero crossing, and over range detection
    char cmd[NAMP*NCMD];
    
    for (i = 0; i < NAMP*NCMD; i++)
      cmd[i] = 0x00;
    
    uint8_t def_config = 0x80;
    uint8_t code = 0x00;
    

    pr_info("Initializing the Audio Logic PGA2505 module\n");

    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&PGA2505_platform);
    if (ret_val != 0)
    {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }

    // Register the device
    struct spi_board_info spi_device_info = {
        .modalias = "fe_PGA2505_",
        .max_speed_hz = speed,
        .bus_num = 0,             // Determined from /sys/class/spi_master
        .chip_select = 1,
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

    printk("Setting the bits per word\n");
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

    // Set the gain to 0 dB
    fill_cmd_array(cmd,encode_gpio(code),def_config);
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));

    pr_info("Audio Logic PGA2505 module successfully initialized!\n");

    return 0;
}



/** Kernel module loading for platform devices

    Called by the kernel when a module is loaded which matches a device tree overlay entry.
    This function does all the setup of the device driver and creates the sysfs entries.

    @param pdev Pointer to a platform_device structure containing information from the overlay about the device to load
    @returns SUCCESS or error code
*/
static int PGA2505_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;

    char deviceName[20] = "fe_PGA2505_";
    char deviceMinor[20];
    int status;

    struct device *deviceObj;
    fe_PGA2505_dev_t *fe_PGA2505_devp;

    pr_info("PGA2505_probe enter\n");

    // Create structure to hold device-specific information (like the registers). Make size of &pdev->dev + sizeof(struct(fe_PGA2505_dev)).
    fe_PGA2505_devp = devm_kzalloc(&pdev->dev, sizeof(fe_PGA2505_dev_t), GFP_KERNEL);

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void *)fe_PGA2505_devp);

    //Create a memory region to store the device name
    fe_PGA2505_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
    if (fe_PGA2505_devp->name == NULL)
        goto bad_mem_alloc;

    //Copy the name from the overlay and stick it in the created memory region
    strcpy(fe_PGA2505_devp->name, (char *)pdev->name);
    pr_info("%s\n", (char *)pdev->name);

    //Request a Major/Minor number for the driver
    status = alloc_chrdev_region(&dev_num, 0, 1, "fe_PGA2505_");
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
    cdev_init(&fe_PGA2505_devp->cdev, &fe_PGA2505_fops);

    //Registers the char driver with the kernel
    status = cdev_add(&fe_PGA2505_devp->cdev, dev_num, 1);
    if (status != 0)
        goto bad_cdev_add;

    //Creates the device entries in sysfs
    deviceObj = device_create(cl, NULL, dev_num, NULL, deviceName);
    if (deviceObj == NULL)
        goto bad_device_create;

    //Put a pointer to the fe_fir_dev struct that is created into the driver object so it can be accessed uniquely from elsewhere
    dev_set_drvdata(deviceObj, fe_PGA2505_devp);

    //---------------------------------------------------------
    status = device_create_file(deviceObj, &dev_attr_volume);
    if (status)
        goto bad_device_create_file_1;

    //---------------------------------------------------------
    status = device_create_file(deviceObj, &dev_attr_name);
    if (status)
        goto bad_device_create_file_2;

    pr_info("PGA2505_probe exit\n");

    return 0;

  bad_device_create_file_2:
      device_remove_file(deviceObj, &dev_attr_name);

  bad_device_create_file_1:
      device_remove_file(deviceObj, &dev_attr_volume);

  bad_device_create:
      cdev_del(&fe_PGA2505_devp->cdev);

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
static int PGA2505_open(struct inode *inode, struct file *file)
{
    //Create a pointer to the driver instance
    fe_PGA2505_dev_t *devp;

    //Put it in the container_of structure so it can be used from anywhere
    devp = container_of(inode->i_cdev, fe_PGA2505_dev_t, cdev);
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
static int PGA2505_release(struct inode *inode, struct file *file)
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
    @returns PGA2505_read Number of bits sent in buffer, and will return 0 for the last transaction.
*/
static ssize_t PGA2505_read(struct file *file, char *buffer, size_t len, loff_t *offset)
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
    @returns PGA2505_read Number of bytes written
*/
static ssize_t PGA2505_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
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
static int PGA2505_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    fe_PGA2505_dev_t *dev = (fe_PGA2505_dev_t *)platform_get_drvdata(pdev);

    pr_info("PGA2505_remove enter\n");

    // Unregister the character file (remove it from /dev)
    cdev_del(&dev->cdev);

    //Tell the os that the major/minor pair is avalible again
    unregister_chrdev_region(dev_num, 2);

    pr_info("PGA2505_remove exit\n");

    return 0;
}



// Called when the driver is removed
static void PGA2505_exit(void)
{
    pr_info("Audio Logic PGA2505 module exit\n");

    // Unregister our driver from the "Platform Driver" bus
    // This will cause "PGA2505_remove" to be called for each connected device
    platform_driver_unregister(&PGA2505_platform);

    if( spi_device ){
        spi_unregister_device( spi_device );
    }

    pr_info("Audio Logic PGA2505 module successfully unregistered\n");
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
    fe_PGA2505_dev_t *devp = (fe_PGA2505_dev_t *)dev_get_drvdata(dev);

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
    int ret_val;
    
    uint8_t def_config = 0x80;
    uint8_t code = 0x00;

    char cmd[NAMP * NCMD];
    
    for (i = 0; i < NAMP*NCMD; i++)
      cmd[i] = 0x00;
    

    // Create a new instance of the PGA
    fe_PGA2505_dev_t *devp = (fe_PGA2505_dev_t *)dev_get_drvdata(dev);

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
    tempValue = set_fixed_num(substring);

    // Determine the code for the volume level
    code = find_volume_level(tempValue);

    // Determine the closest volume level
    tempValue = decode_volume(code);

    // Record the volume level to the volume variable
    devp->volume = tempValue;

    // Populate the SPI command and send it
    fill_cmd_array(cmd,encode_gpio(code),def_config);
    ret_val = spi_write(spi_device,&cmd, sizeof(cmd));

    return count;
}
static ssize_t volume_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    fe_PGA2505_dev_t *devp = (fe_PGA2505_dev_t *)dev_get_drvdata(dev);

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

/** Converts a 32 bit integer into an 8 bit volume level
    @param fp28_num 32 bit representation of a fixed point number
    @return volume_level an 8 bit representation of the attenuation
*/
uint8_t find_volume_level(uint32_t fp28_num)
{
  // Initialize the volume level
  uint8_t volume_level = 0;

  // Divide the input number by the reciprocal of the step size
  uint32_t inputnum = (fp28_num*10)/3;

  // Find the integer portion and the tenths portion
  // (Steps are in 3/8 increments so only that position is needed)
  uint16_t upperval = (uint16_t)((inputnum)>>16)/10;
  uint16_t lowerval = inputnum-upperval*10;

  // Catch whether the user exceeded the maximum attenuation
  // This number includes the 6 dB offset (2)
  if (upperval > 20)
  {
    // Set the upper value to the 60 dB code (18 is the start)
    upperval = 18;
    lowerval = 0;
    printk("Input exceeds the maximum amplification of 60 dB.\n");
    printk("Setting amplification to 60 dB.\n");
  }
  // If the input is less than one step below the minimum attenuation,
  // set the code to zero
  else if (upperval < 2)
  {
    upperval = 0;
    lowerval = 0;
    printk("Input is below minimum amplification of 6 dB.\n");
    printk("Setting amplification to 9 dB.\n");
  }
  // Otherwise, the gain is between 9 and 60 dB
  else
    upperval = upperval - 2; // Subtract the 6 dB offset

  // Set the volume level to the integer portion
  volume_level = (uint8_t)(upperval);

  // If the tenths portion is greater than five, round up
  if (lowerval > 5)
    volume_level += 1;

  return volume_level;
}

/** Converts an 8 bit volume level representation to a 32 bit fixed point 28 number
    @param volume_level an 8 bit representation of the attenuation
    @return fp28_num 32 bit representation of a fixed point number
*/
uint32_t decode_volume(uint8_t code)
{

  uint32_t decodedDb;
  // Cast the volume level to a 32 bit number
  if (code > 1)
    decodedDb = (uint32_t)code*3+6;
  else
    decodedDb = 0;

  // Return the shifted and scaled number to keep with convention
  return (decodedDb<<16);
}


/** Converts an 8 bit volume level representation to a 6 bit LED code.  This code is
    tied to the hardware configuration of the AD1939 Expansion card for the Audio 
    Blade.  The mapping of GPIO to LEDs is as follows:

    ------------------------------------------------------
    | GPIO | U8.2 | U8.1 | U10.2 | U10.1 | U10.4 | U10.3 |
    |-----------------------------------------------------
    | LED  | LED1 | LED2 | LED3  | LED4  | LED5  | LED6  |
    ------------------------------------------------------

    @param code an 8 bit representation of the attenuation
    @return 8 bit representation of and LED gain level
*/
uint8_t encode_gpio(uint8_t code)
{
  // Divide the range of 1 to 18 relatively evenly
  if (code == 0)
    return 0x80;
  else if (code < 4)
    return 0xA0;
  else if (code < 7)
    return 0xB0;
  else if (code < 10)
    return 0xB2;
  else if (code < 13)
    return 0xB3;
  else if (code < 16)
    return 0xBB;
  else
    return 0xBF;
}

/** Custom function to fill the command array to map the custom GPIO behaviors.
    This function should be overwritten for other designs.

   @param code an 8 bit integer representing the GPIO configuration
   @param defreg an 8 bit integer representing the default configuration of the PGA2505
*/
void fill_cmd_array(char* cmd, uint8_t code,uint8_t defreg)
{
    int i;
    uint8_t bitmask = 0x0F;
    

    // Set the default configuration for all PGA2505s in the chain
    for (i=0; i < NAMP; i++)
      cmd[i*NCMD] = cmd[i*NCMD] | defreg;
    
    cmd[1] = cmd[1] | (bitmask & code);
    cmd[3] = cmd[3] | code>>4;
}

/** Tell the kernel what the initialization function is */
module_init(PGA2505_init);

/** Tell the kernel what the delete function is */
module_exit(PGA2505_exit);
