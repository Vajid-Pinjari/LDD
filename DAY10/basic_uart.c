#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/serial_reg.h>
#include <linux/io.h>

#define UART_BASE_ADDR 0x3F215040 // Base address for UART0 (adjust if needed for your Pi model)
#define DEVICE_NAME "simple_uart"
#define BUF_SIZE 1

static dev_t dev;
static struct cdev uart_cdev;
static struct class *uart_class;
static char char_buffer[BUF_SIZE];

static int uart_open(struct inode *inode, struct file *file) {
    return 0;
}

static int uart_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t uart_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    while (!(ioread8(UART_BASE_ADDR + UART_LSR) & UART_LSR_DR)); // Wait for data
    char_buffer[0] = ioread8(UART_BASE_ADDR + UART_RX);
    if (copy_to_user(buf, char_buffer, BUF_SIZE))
        return -EFAULT;
    return BUF_SIZE;
}

static ssize_t uart_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    if (copy_from_user(char_buffer, buf, BUF_SIZE))
        return -EFAULT;
    while (!(ioread8(UART_BASE_ADDR + UART_LSR) & UART_LSR_THRE)); // Wait for space
    iowrite8(char_buffer[0], UART_BASE_ADDR + UART_TX);
    return BUF_SIZE;
}

static const struct file_operations uart_fops = {
    .owner = THIS_MODULE,
    .open = uart_open,
    .release = uart_release,
    .read = uart_read,
    .write = uart_write,
};

static int __init uart_init(void) {
    int ret;

    // Register a character device
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate char device region\n");
        return ret;
    }

    cdev_init(&uart_cdev, &uart_fops);
    uart_cdev.owner = THIS_MODULE;

    ret = cdev_add(&uart_cdev, dev, 1);
    if (ret < 0) {
        printk(KERN_ERR "Failed to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    uart_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(uart_class)) {
        printk(KERN_ERR "Failed to create device class\n");
        cdev_del(&uart_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(uart_class);
    }

    device_create(uart_class, NULL, dev, NULL, DEVICE_NAME);
    printk(KERN_INFO "UART driver initialized\n");
    return 0;
}

static void __exit uart_exit(void) {
    device_destroy(uart_class, dev);
    class_destroy(uart_class);
    cdev_del(&uart_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "UART driver removed\n");
}

module_init(uart_init);
module_exit(uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vajid");
MODULE_DESCRIPTION("Simple UART Driver");
