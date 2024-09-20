#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Travis Reynertson");
MODULE_DESCRIPTION("ECEn 427 pit Driver");

#define MODULE_NAME "pit"

#define PIT_CODEC_DEVICE_NAME "pit_codec"

// Define the memory region size and virtual base address for the pit codec
#define PIT_MEM_SIZE 0x1000

#define PIT_BUFFER_SIZE (512 * 1024)

#define INIT_FUNCTION_NAME "pit_init"
#define EXIT_FUNCTION_NAME "pit_exit"
#define PROBE_FUNCTION_NAME "pit_probe"
#define PIT_REMOVE_FUNCTION_NAME "pit_remove"
#define PIT_READ_FUNCTION_NAME "pit_read"
#define PIT_WRITE_FUNCTION_NAME "pit_write"


////////////////////////////////////////////////////////////////////////////////
/////////////////////// Forward function declarations //////////////////////////
////////////////////////////////////////////////////////////////////////////////
static int pit_init(void);
static void pit_exit(void);
static int pit_probe(struct platform_device *pdev);
static int pit_remove(struct platform_device *pdev);

enum cleanup_level {
  CLEANUP_ALL,
  CLEANUP_MEM,
  CLEANUP_RELEASE_MEM,
  CLEANUP_PIT_KOBJ,
  CLEANUP_SYSFS_GROUP
};

void pit_probe_cleanup(enum cleanup_level level);
static uint32_t pit_read_reg(uint32_t offset);
static void pit_write_reg(uint32_t offset, uint32_t value);

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Struct //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct pit_device {           
  struct platform_device *pdev; // Platform device pointer
  phys_addr_t phys_addr;        // Physical address
  u32 mem_size;                 // Allocated mem space size
  u32 *virt_addr;               // Virtual address
};

static struct of_device_id pit_match[] = {
    {
        .compatible = "byu,ecen427-pit",
    },
    {}};

static struct platform_driver pit_platform_driver = {
    .probe = pit_probe,
    .remove = pit_remove,
    .driver =
        {
            .name = MODULE_NAME,
            .owner = THIS_MODULE,
            .of_match_table = pit_match,
        },
};


///////////////////////////////////////////////////////////////////////////////
/////////////////////// sysfs attribute structures ////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static ssize_t show_period(struct device *dev, struct device_attribute *attr, char *buf) {
    dev_info(dev, "In show period\n");
    dev_info(dev, "buf value is %s\n", buf);	
    return snprintf(buf, 24, "%u", pit_read_reg(0x4)/100);
}

static ssize_t store_period(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {	
    dev_info(dev, "In store period\n");	 
    dev_info(dev, "buf value is %s\n", buf);
    pit_write_reg(0x4, (uint32_t)simple_strtoul(buf, NULL, 10) * 100);
    return count;
}

static ssize_t show_run(struct device *dev, struct device_attribute *attr, char *buf) {
    return 0x1 & snprintf(buf, PAGE_SIZE, "%u", pit_read_reg(0x0000));
}

static ssize_t store_run(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    pit_write_reg(0x0000, 0x1 & (uint32_t)simple_strtoul(buf, NULL, 10));
    return count;
}

static ssize_t show_int_en(struct device *dev, struct device_attribute *attr, char *buf) {
    return 0x2 & snprintf(buf, PAGE_SIZE, "%u", pit_read_reg(0x0000));
}

static ssize_t store_int_en(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    pit_write_reg(0x0000, 0x2 & (uint32_t)simple_strtoul(buf, NULL, 10)); // Assuming the same register is used for "int_en"
    return count;
}

static struct device_attribute pit_period_attr = {
	.attr = {
		.name = "period",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_period,
	.store = store_period,
};

static struct device_attribute pit_run_attr = {
	.attr = {
		.name = "run",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_run,
	.store = store_run,
};

static struct device_attribute pit_int_en_attr = {
	.attr = {
		.name = "int_en",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_int_en,
	.store = store_int_en,
};

// Define attribute group
static struct attribute *pit_attrs[] = {
    &pit_period_attr.attr,
    &pit_run_attr.attr,
    &pit_int_en_attr.attr,
    NULL,
};

static const struct attribute_group pit_attr_group = {
    .attrs = pit_attrs,
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Globals /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// The pit device - since this driver only supports one device, we don't
// need a list here, we can just use a single struct.
static struct pit_device pit;

static struct kobject *pit_kobj = NULL;

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This section contains driver-level functions.

// Register driver init/exit with Linux
module_init(pit_init);
module_exit(pit_exit);

static int pit_init(void) {

  pr_info("%s: Initializing pit Driver in pit_init!\n", INIT_FUNCTION_NAME);

  // Register the driver as a platform driver -- platform_driver_register
  int result = platform_driver_register(&pit_platform_driver);
  if (result < 0) {
    pr_err("%s: Failed to register platform driver\n", INIT_FUNCTION_NAME);
    return -EINVAL;
  }

  pr_info("%s: pit Driver Initialization Complete in pit_init!\n",
          INIT_FUNCTION_NAME);

  return 0;
}

// This is called when Linux unloads your driver
static void pit_exit(void) {
  pr_info("%s: Removing Pit Driver in pit_exit!\n", EXIT_FUNCTION_NAME);

  // platform_driver_unregister
  platform_driver_unregister(&pit_platform_driver);

  pr_info("%s: Pit Driver Removed in pit_exit!\n", EXIT_FUNCTION_NAME);
  return;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void pit_probe_cleanup(enum cleanup_level level) {
  switch (level) {
  case CLEANUP_ALL:
  case CLEANUP_SYSFS_GROUP:
    sysfs_remove_group(pit_kobj, &pit_attr_group);
    /* Fall through */
  case CLEANUP_PIT_KOBJ:
    // Delete the kobject
    kobject_put(pit_kobj);
    /* Fall through */
  case CLEANUP_MEM:
    iounmap(pit.virt_addr);
    /* Fall through */
  case CLEANUP_RELEASE_MEM:
    release_mem_region(pit.phys_addr, PIT_MEM_SIZE);
    break;
  default:
    dev_err(&pit.pdev->dev, "%s: Failed to clean up probe\n",
            PROBE_FUNCTION_NAME);
    break;
  }
}

static int pit_probe(struct platform_device *pdev) {
    pit.pdev = pdev;

    // Print debug information
    pr_info("%s: Starting pit_probe!\n", PROBE_FUNCTION_NAME);

    // Get the physical device address from the device tree
    struct resource *res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res_mem) {
        dev_err(&pdev->dev, "%s: Failed to get memory resource\n", MODULE_NAME);
        return -ENODEV;
    }
    pit.phys_addr = res_mem->start;
    pit.mem_size = resource_size(res_mem);

    dev_info(&pdev->dev, "phys_addr is: %d\n", pit.phys_addr);

    // Reserve the memory region
    if (!request_mem_region(pit.phys_addr, PIT_MEM_SIZE, MODULE_NAME)) {
        dev_err(&pdev->dev, "%s: Failed to reserve memory region\n", MODULE_NAME);
        return -EBUSY;
    }

    // Get a (virtual memory) pointer to the device
    pit.virt_addr = ioremap(pit.phys_addr, PIT_MEM_SIZE);
    if (!pit.virt_addr) {
        dev_err(&pdev->dev, "%s: Failed to map memory region\n", MODULE_NAME);
        pit_probe_cleanup(CLEANUP_RELEASE_MEM);
        return -ENOMEM;
    }

    dev_info(&pdev->dev, "virt_addr is: %p\n", (void *)pit.virt_addr);

    // Register the attribute group with sysfs
    pit_kobj = &pdev->dev.kobj;
    if (sysfs_create_group(pit_kobj, &pit_attr_group)) {
        pit_probe_cleanup(CLEANUP_PIT_KOBJ);
        return -ENOMEM;;
    }

    pit_write_reg(0x0, 0x3);
    pit_write_reg(0x4, 0x60000000);

    // uint32_t num = pit_read_reg(0x0);
    // dev_info(&pdev->dev, "num is: %d\n", num);

    // uint32_t period = pit_read_reg(0x4);
    // dev_info(&pdev->dev, "Period is: %d\n", period);

    // Print debug information
    dev_info(&pdev->dev, "Finish pit_probe!\n");

    return 0; //(success)
}

// Called when the platform device is removed
static int pit_remove(struct platform_device *pdev) {
  // iounmap
  // release_mem_region
  dev_info(&pdev->dev, "%s: Start pit_remove function!\n",
           PIT_REMOVE_FUNCTION_NAME);

  pit_probe_cleanup(CLEANUP_ALL);

  dev_info(&pdev->dev, "%s: Finish pit_remove function!\n",
           PIT_REMOVE_FUNCTION_NAME);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// File Operations ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Reads a register from the pit device
static uint32_t pit_read_reg(uint32_t offset) {
    return ioread32(pit.virt_addr + (offset / sizeof(uint32_t)));
}

// Writes a value to a register in the pit device
static void pit_write_reg(uint32_t offset, uint32_t value) {
    iowrite32(value, pit.virt_addr + (offset / sizeof(uint32_t)));
}
