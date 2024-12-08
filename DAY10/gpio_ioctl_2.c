#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "gpio_ioctl"
#define CLASS_NAME "gpio_class"

// Define GPIO pin (change as needed for Raspberry Pi)
#define GPIO_PIN 18

// IOCTL commands
#define GPIO_MAGIC 'G'
#define GPIO_CMD_SET_PIN _IOW(GPIO_MAGIC, 0, int)
#define GPIO_CMD_GET_PIN _IOR(GPIO_MAGIC, 1, int)

static int major_number;
static struct class *gpio_class = NULL;
static struct device *gpio_device = NULL;
static struct cdev gpio_cdev;

// Open function
static int gpio_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "GPIO Device: Opened\n");
    return 0;
}

// Release function
static int gpio_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "GPIO Device: Closed\n");
    return 0;
}

// IOCTL handler
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int value;

    switch (cmd) {
        case GPIO_CMD_SET_PIN:
            if (copy_from_user(&value, (int __user *)arg, sizeof(value))) {
                return -EFAULT;
            }
            gpio_set_value(GPIO_PIN, value);
            printk(KERN_INFO "GPIO Device: Set GPIO_PIN %d to %d\n", GPIO_PIN, value);
            break;

        case GPIO_CMD_GET_PIN:
            value = gpio_get_value(GPIO_PIN);
            if (copy_to_user((int __user *)arg, &value, sizeof(value))) {
                return -EFAULT;
            }
            printk(KERN_INFO "GPIO Device: Get GPIO_PIN %d value %d\n", GPIO_PIN, value);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

// File operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .release = gpio_release,
    .unlocked_ioctl = gpio_ioctl,
};

// Module initialization
static int __init gpio_ioctl_init(void) {
    int ret;

    // Allocate a major number dynamically
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "GPIO Device: Failed to register a major number\n");
        return major_number;
    }

    // Create device class
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "GPIO Device: Failed to create device class\n");
        return PTR_ERR(gpio_class);
    }

    // Create device
    gpio_device = device_create(gpio_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(gpio_device)) {
        class_destroy(gpio_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "GPIO Device: Failed to create device\n");
        return PTR_ERR(gpio_device);
    }

    // Initialize GPIO pin
    if (!gpio_is_valid(GPIO_PIN)) {
        printk(KERN_ERR "GPIO Device: Invalid GPIO_PIN %d\n", GPIO_PIN);
        device_destroy(gpio_class, MKDEV(major_number, 0));
        class_destroy(gpio_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return -ENODEV;
    }

    gpio_request(GPIO_PIN, "sysfs");
    gpio_direction_output(GPIO_PIN, 0); // Default output low
    gpio_export(GPIO_PIN, false);

    printk(KERN_INFO "GPIO Device: Driver initialized successfully\n");
    return 0;
}

// Module cleanup
static void __exit gpio_ioctl_exit(void) {
    gpio_unexport(GPIO_PIN);
    gpio_free(GPIO_PIN);
    device_destroy(gpio_class, MKDEV(major_number, 0));
    class_destroy(gpio_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "GPIO Device: Driver exited successfully\n");
}

module_init(gpio_ioctl_init);
module_exit(gpio_ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vajid");
MODULE_DESCRIPTION("GPIO Control Driver using IOCTL for Raspberry Pi");
