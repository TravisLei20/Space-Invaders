#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Travis Reynertson");
MODULE_DESCRIPTION("ECEn 427 Audio Driver");

#define MODULE_NAME "audio"

#define AUDIO_CODEC_DEVICE_NAME "audio_codec"

// Define the memory region size and virtual base address for the audio codec
#define AUDIO_MEM_SIZE 0x1000

#define AUDIO_BUFFER_SIZE (512 * 1024)

#define INIT_FUNCTION_NAME "audio_init"
#define EXIT_FUNCTION_NAME "audio_exit"
#define PROBE_FUNCTION_NAME "audio_probe"
#define AUDIO_REMOVE_FUNCTION_NAME "audio_remove"
#define AUDIO_READ_FUNCTION_NAME "audio_read"
#define AUDIO_WRITE_FUNCTION_NAME "audio_write"
#define AUDIO_IRQ_HANDLER_FUNCTION_NAME "audio_irq_handler"

// Register offsets and bit masks for I2S_STATUS_REG
#define I2S_STATUS_REG_OFFSET 0x10 // Example offset, replace with actual offset
#define I2S_STATUS_REG_DATA_RDY_BIT 21
#define I2S_STATUS_REG_INT_EN_BIT 0
#define I2S_DATA_TX_R_REG_OFFSET 0x0C
#define I2S_DATA_TX_L_REG_OFFSET 0x08

#define SAMPLES_PER_INTERRUPT 768

#define AUDIO_IOCTL_SET_LOOP _IO('A', 0)
#define AUDIO_IOCTL_CLEAR_LOOP _IO('A', 1)


////////////////////////////////////////////////////////////////////////////////
/////////////////////// Forward function declarations //////////////////////////
////////////////////////////////////////////////////////////////////////////////
static int audio_init(void);
static void audio_exit(void);
static int audio_probe(struct platform_device *pdev);
static int audio_remove(struct platform_device *pdev);
static ssize_t audio_read(struct file *file, char __user *buf, size_t count,
                          loff_t *offset);
static ssize_t audio_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *offset);
enum cleanup_level {
  CLEANUP_ALL,
  CLEANUP_MEM,
  CLEANUP_IRQ,
  CLEANUP_RELEASE_MEM,
  CLEANUP_DEV,
  CLEANUP_CDEV
};
void audio_probe_cleanup(enum cleanup_level level);
irqreturn_t audio_irq_handler(int irq, void *dev_id);
// static uint32_t audio_read_reg(uint32_t offset);
static void audio_write_reg(uint32_t offset, uint32_t value);
long audio_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
void audio_set_looping(bool enable);


////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Struct //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct audio_device {
  int minor_num;                // Device minor number
  struct cdev cdev;             // Character device structure
  struct platform_device *pdev; // Platform device pointer
  struct device *dev;           // device (/dev)
  phys_addr_t phys_addr;        // Physical address
  u32 mem_size;                 // Allocated mem space size
  u32 *virt_addr;               // Virtual address

  // Add any device-specific items to this that you need
  int32_t audio_buffer[AUDIO_BUFFER_SIZE];
  int audio_buffer_index; // Index to keep track of the audio buffer
  int audio_clip_size;
  bool looping_enabled;
};

static struct of_device_id audio_match[] = {
    {
        .compatible = "byu,ecen427-audio_codec",
    },
    {}};

static struct platform_driver audio_platform_driver = {
    .probe = audio_probe,
    .remove = audio_remove,
    .driver =
        {
            .name = MODULE_NAME,
            .owner = THIS_MODULE,
            .of_match_table = audio_match,
        },
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Globals /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Major number to use for devices managed by this driver
static int major_num;

// IRQ number
static int irq;

// Device number
static dev_t devNum = MKDEV(0, 0);

// The audio device - since this driver only supports one device, we don't
// need a list here, we can just use a single struct.
static struct audio_device audio;

// The device class
static struct class *created_class;

// File operations structure
static const struct file_operations audio_fops = {
    .owner = THIS_MODULE,
    .read = audio_read,
    .write = audio_write,
    .unlocked_ioctl = audio_ioctl,
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This section contains driver-level functions.

// Register driver init/exit with Linux
module_init(audio_init);
module_exit(audio_exit);

static int audio_init(void) {

  pr_info("%s: Initializing Audio Driver in audio_init!\n", INIT_FUNCTION_NAME);

  // pr_info("%s: Get Major Number in audio_init!\n", INIT_FUNCTION_NAME);
  // dev_t devNum
  // Get a major number for the driver -- alloc_chrdev_region; // pg. 45, LDD3.
  int result = alloc_chrdev_region(&devNum, 0, 1, MODULE_NAME);
  if (result < 0) {
    pr_err("%s: Failed to allocate major number\n", INIT_FUNCTION_NAME);
    return -ENOMEM;
  }

  major_num = MAJOR(devNum);
  audio.minor_num = MINOR(devNum);

  pr_info("%s: Major Number is: %d\n", INIT_FUNCTION_NAME, major_num);
  pr_info("%s: Minor Number is: %d\n", INIT_FUNCTION_NAME, audio.minor_num);

  // Create a device class. -- class_create()
  created_class = class_create(THIS_MODULE, MODULE_NAME);
  if (IS_ERR(created_class)) {
    pr_err("%s: Failed to create device class\n", INIT_FUNCTION_NAME);
    unregister_chrdev_region(devNum, 1);
    return PTR_ERR(created_class);
  }

  // Register the driver as a platform driver -- platform_driver_register
  result = platform_driver_register(&audio_platform_driver);
  if (result < 0) {
    pr_err("%s: Failed to register platform driver\n", INIT_FUNCTION_NAME);
    class_destroy(created_class);
    unregister_chrdev_region(devNum, 1);
    return -EINVAL;
  }

  // Clear the audio buffer
  memset(audio.audio_buffer, 0, AUDIO_BUFFER_SIZE * sizeof(int32_t));
  audio.audio_buffer_index = 0;
  audio.audio_clip_size = 0;

  pr_info("%s: Audio Driver Initialization Complete in audio_init!\n",
          INIT_FUNCTION_NAME);

  return 0;
}

// This is called when Linux unloads your driver
static void audio_exit(void) {
  pr_info("%s: Removing Audio Driver in audio_exit!\n", EXIT_FUNCTION_NAME);
  // platform_driver_unregister
  // class_destroy
  // unregister_chrdev_region
  platform_driver_unregister(&audio_platform_driver);
  class_destroy(created_class);
  unregister_chrdev_region(devNum, 1);

  pr_info("%s: Audio Driver Removed in audio_exit!\n", EXIT_FUNCTION_NAME);
  return;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This section contains device-level functions.

void audio_probe_cleanup(enum cleanup_level level) {
  switch (level) {
  case CLEANUP_ALL:
  case CLEANUP_IRQ:
    free_irq(irq, &audio);
    /* Fall through */
  case CLEANUP_MEM:
    iounmap(audio.virt_addr);
    /* Fall through */
  case CLEANUP_RELEASE_MEM:
    release_mem_region(audio.phys_addr, AUDIO_MEM_SIZE);
    /* Fall through */
  case CLEANUP_DEV:
    device_destroy(created_class, devNum);
    /* Fall through */
  case CLEANUP_CDEV:
    cdev_del(&audio.cdev);
    break;
  default:
    dev_err(&audio.pdev->dev, "%s: Failed to clean up probe\n",
            PROBE_FUNCTION_NAME);
    break;
  }
}

irqreturn_t audio_irq_handler(int irq, void *dev_id) {

  irqreturn_t ret = IRQ_HANDLED;

  // Disable interrupts
  audio_write_reg(I2S_STATUS_REG_OFFSET, 0);

  for (int i = 0; i < SAMPLES_PER_INTERRUPT; i++) {
    if (audio.audio_buffer_index >= audio.audio_clip_size) {
      if (audio.looping_enabled) {
        audio.audio_buffer_index = 0;
      } else {
        return ret;
      }
    }
    audio_write_reg(I2S_DATA_TX_R_REG_OFFSET,
                    audio.audio_buffer[audio.audio_buffer_index]);
    audio_write_reg(I2S_DATA_TX_L_REG_OFFSET,
                    audio.audio_buffer[audio.audio_buffer_index]);
    audio.audio_buffer_index++;
  }

  // Re-enable interrupts
  audio_write_reg(I2S_STATUS_REG_OFFSET, 1);

  return ret;
}

static int audio_probe(struct platform_device *pdev) {
  audio.pdev = pdev;

  // Print debug information
  pr_info("%s: Starting audio_probe!\n", PROBE_FUNCTION_NAME);

  // Initialize the character device structure (cdev_init)
  cdev_init(&audio.cdev, &audio_fops);
  audio.cdev.owner = THIS_MODULE;

  // Register the character device with Linux (cdev_add)
  if (cdev_add(&audio.cdev, devNum, 1) < 0) {
    dev_err(&pdev->dev, "%s: Failed to add character device\n",
            PROBE_FUNCTION_NAME);
    return -EINVAL;
  }

  // Create a device file in /dev so that the character device can be accessed
  // from user space (device_create).
  audio.dev = device_create(created_class, &pdev->dev, devNum, NULL,
                            AUDIO_CODEC_DEVICE_NAME);
  if (IS_ERR(audio.dev)) {
    dev_err(&pdev->dev, "Failed to create device file\n");
    audio_probe_cleanup(CLEANUP_CDEV);
    return PTR_ERR(audio.dev);
  }

  // Get the physical device address from the device tree
  struct resource *res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!res_mem) {
    dev_err(&pdev->dev, "%s: Failed to get memory resource\n", MODULE_NAME);
    audio_probe_cleanup(CLEANUP_DEV);
    return -ENODEV;
  }
  audio.phys_addr = res_mem->start;
  audio.mem_size = resource_size(res_mem);

  dev_info(&pdev->dev, "phys_addr is: %d\n", audio.phys_addr);

  // Reserve the memory region
  if (!request_mem_region(audio.phys_addr, AUDIO_MEM_SIZE, MODULE_NAME)) {
    dev_err(&pdev->dev, "%s: Failed to reserve memory region\n", MODULE_NAME);
    audio_probe_cleanup(CLEANUP_DEV);
    return -EBUSY;
  }

  // Get a (virtual memory) pointer to the device
  audio.virt_addr = ioremap(audio.phys_addr, AUDIO_MEM_SIZE);
  if (!audio.virt_addr) {
    dev_err(&pdev->dev, "%s: Failed to map memory region\n", MODULE_NAME);
    audio_probe_cleanup(CLEANUP_RELEASE_MEM);
    return -ENOMEM;
  }

  dev_info(&pdev->dev, "virt_addr is: %p\n", (void *)audio.virt_addr);

  // Get the IRQ number from the device tree
  irq = platform_get_irq(pdev, 0);
  if (irq < 0) {
    dev_err(&pdev->dev, "%s: Failed to get IRQ number\n", MODULE_NAME);
    audio_probe_cleanup(CLEANUP_MEM);
    return irq;
  }
  dev_info(&pdev->dev, "irq is: %d\n", irq);

  // Register interrupt service routine
  int irq_result =
      request_irq(irq, audio_irq_handler, IRQF_SHARED, MODULE_NAME, &audio);
  if (irq_result) {
    dev_err(&pdev->dev, "%s: Failed to register IRQ handler\n",
            PROBE_FUNCTION_NAME);
    audio_probe_cleanup(CLEANUP_MEM);
    return -EBUSY;
  }

  // Disable interrupts
  audio_write_reg(I2S_STATUS_REG_OFFSET, 0);

  // Disable looping by default
  audio_set_looping(false);

  // Print debug information
  dev_info(&pdev->dev, "Finish audio_probe!\n");

  return 0; //(success)
}

// Called when the platform device is removed
static int audio_remove(struct platform_device *pdev) {
  // free_irq
  // iounmap
  // release_mem_region
  // device_destroy
  // cdev_del
  dev_info(&pdev->dev, "%s: Start audio_remove function!\n",
           AUDIO_REMOVE_FUNCTION_NAME);

  audio_probe_cleanup(CLEANUP_ALL);

  dev_info(&pdev->dev, "%s: Finish audio_remove function!\n",
           AUDIO_REMOVE_FUNCTION_NAME);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// File Operations ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Reads a register from the audio device
// static uint32_t audio_read_reg(uint32_t offset) {
//     return ioread32(audio.virt_addr + offset);
// }

// Writes a value to a register in the audio device
static void audio_write_reg(uint32_t offset, uint32_t value) {
  iowrite32(value, audio.virt_addr + (offset / sizeof(uint32_t)));
}

// Read callback function
static ssize_t audio_read(struct file *file, char __user *buf, size_t count, loff_t *offset) {
  return audio.audio_buffer_index + 1 < audio.audio_clip_size ? 1 : 0;
}

// Write callback function
static ssize_t audio_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *offset) {
  ssize_t ret = 0;

  pr_info("%s: In the audio_write function!\n", AUDIO_WRITE_FUNCTION_NAME);

  audio.audio_buffer_index = 0;
  audio.audio_clip_size = count;
  memset(audio.audio_buffer, 0, AUDIO_BUFFER_SIZE * sizeof(int32_t));

  // Disable interrupts from the audio core
  audio_write_reg(I2S_STATUS_REG_OFFSET, 0);

  // Check if the user space buffer is valid
  if (!buf) {
    pr_err("%s: Invalid user space buffer\n", AUDIO_WRITE_FUNCTION_NAME);
    ret = -EFAULT;
    goto enable_interrupts;
  }

  // Check if the count is valid
  if (count > AUDIO_BUFFER_SIZE) {
    pr_err("%s: Audio clip size exceeds buffer size\n",
           AUDIO_WRITE_FUNCTION_NAME);
    ret = -EINVAL;
    goto enable_interrupts;
  }

  // Copy audio data from user space to kernel space buffer
  if (copy_from_user(audio.audio_buffer, buf, count) != 0) {
    pr_err("%s: Failed to copy data from user space\n",
           AUDIO_WRITE_FUNCTION_NAME);
    ret = -EFAULT;
    goto enable_interrupts;
  }

  // pr_info("%d, %d, %d, %d, %d\n", audio.audio_buffer[0], audio.audio_buffer[1],
  //         audio.audio_buffer[2], audio.audio_buffer[3], audio.audio_buffer[4]);

  // Re-enable interrupts on the audio core
enable_interrupts:
  audio_write_reg(I2S_STATUS_REG_OFFSET, 1);

  return ret == 0 ? count : ret;
}

long audio_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret = 0;

    switch (cmd) {
        case AUDIO_IOCTL_SET_LOOP:
            audio_set_looping(true);
            break;
        case AUDIO_IOCTL_CLEAR_LOOP:
            audio_set_looping(false);
            break;
        default:
            ret = -ENOTTY;
            break;
    }

    return ret;
}

void audio_set_looping(bool enable) {
    audio.looping_enabled = enable;
}
