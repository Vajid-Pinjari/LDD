#include<linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "simple_device"
#define BUFFER_SIZE 1024

static int major_number;
static char device_buffer[BUFFER_SIZE] = "Hello from Kernel!\n";
static int device_open = 0;

static int device_open_func(struct inode *inode, struct file *file) {
    if (device_open) {
        return -EBUSY;
    }
    device_open++;
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int device_release_func(struct inode *inode, struct file *file) {
    device_open--;
    printk(KERN_INFO "Device released\n");
    return 0;
}

static ssize_t device_read_func(struct file *filp, char __user *buffer, size_t len, loff_t *offset) {
    size_t data_len = strlen(device_buffer);

    if (*offset >= data_len) {
        return 0;
    }

    if (len > data_len - *offset) {
        len = data_len - *offset;
    }

    if (copy_to_user(buffer, device_buffer + *offset, len)) {
        return -EFAULT;
    }

    *offset += len;
    printk(KERN_INFO "Device read: %zu bytes\n", len);
    return len;
}

static struct file_operations fops = {
    .open = device_open_func,
    .release = device_release_func,
    .read = device_read_func,
};

static int __init simple_driver_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return major_number;
    }

    printk(KERN_INFO "Device registered: %s with major number %d\n", DEVICE_NAME, major_number);
    return 0;
}

static void __exit simple_driver_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Device unregistered\n");
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant Soni");
MODULE_DESCRIPTION("A simple character device driver");
