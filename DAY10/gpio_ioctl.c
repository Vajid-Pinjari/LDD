/*************************//**
 *  \file       gpio_ioctl.c
 *  \details    GPIO driver using ioctl interface for Raspberry Pi
 *  \author     VAJID
 ***************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define DEVICE_NAME "gpio_ioctl"
#define GPIO_PIN    515 // GPIO pin number to control

// ioctl command definitions
#define GPIO_MAGIC     'G'
#define GPIO_SET_DIR   _IOW(GPIO_MAGIC, 0, int)  // Set direction (input/output)
#define GPIO_WRITE     _IOW(GPIO_MAGIC, 1, int)  // Write value to GPIO
#define GPIO_READ      _IOR(GPIO_MAGIC, 2, int)  // Read value from GPIO

static dev_t dev;
static struct cdev gpio_cdev;

// Prototypes
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int gpio_open(struct inode *inode, struct file *file);
static int gpio_release(struct inode *inode, struct file *file);

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = gpio_ioctl,
    .open           = gpio_open,
    .release        = gpio_release,
};

/*
** Open the device
*/
static int gpio_open(struct inode *inode, struct file *file)
{
    pr_info("GPIO device opened\n");
    return 0;
}

/*
** Release the device
*/
static int gpio_release(struct inode *inode, struct file *file)
{
    pr_info("GPIO device closed\n");
    return 0;
}

/*
** ioctl handler
*/
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;
    switch (cmd) {
    case GPIO_SET_DIR:
        if (arg == 0) {
            gpio_direction_input(GPIO_PIN);
            pr_info("Set GPIO %d as input\n", GPIO_PIN);
        } else {
            gpio_direction_output(GPIO_PIN, 0);
            pr_info("Set GPIO %d as output\n", GPIO_PIN);
        }
        break;

    case GPIO_WRITE:
        if (gpio_direction_output(GPIO_PIN, 0)) {
            pr_err("Failed to set GPIO %d as output\n", GPIO_PIN);
            return -EINVAL;
        }
        if (copy_from_user(&value, (int __user *)arg, sizeof(value))) {
            return -EFAULT;
        }
        gpio_set_value(GPIO_PIN, value);
        pr_info("Written %d to GPIO %d\n", value, GPIO_PIN);
        break;

    case GPIO_READ:
        value = gpio_get_value(GPIO_PIN);
        if (copy_to_user((int __user *)arg, &value, sizeof(value))) {
            return -EFAULT;
        }
        pr_info("Read value %d from GPIO %d\n", value, GPIO_PIN);
        break;

    default:
        pr_err("Invalid ioctl command\n");
        return -EINVAL;
    }
    return 0;
}

/*
** Module init function
*/
static int __init gpio_driver_init(void)
{
    // Allocate device number
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        pr_err("Failed to allocate device number\n");
        return -1;
    }

    // Initialize and add cdev
    cdev_init(&gpio_cdev, &fops);
    if (cdev_add(&gpio_cdev, dev, 1) < 0) {
        pr_err("Failed to add cdev\n");
        goto r_unregister;
    }

    // Request GPIO pin
    if (gpio_request(GPIO_PIN, "gpio_ioctl") < 0) {
        pr_err("Failed to request GPIO pin\n");
        goto r_cdev_del;
    }

    pr_info("GPIO ioctl driver loaded successfully\n");
    return 0;

r_cdev_del:
    cdev_del(&gpio_cdev);
r_unregister:
    unregister_chrdev_region(dev, 1);
    return -1;
}

/*
** Module exit function
*/
static void __exit gpio_driver_exit(void)
{
    gpio_free(GPIO_PIN);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("GPIO ioctl driver unloaded\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vajid");
MODULE_DESCRIPTION("GPIO driver using ioctl interface for Raspberry Pi");
MODULE_VERSION("1.0");
