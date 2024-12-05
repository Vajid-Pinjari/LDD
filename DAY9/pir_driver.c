#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

#define GPIO_OUT 17  // Pin number on Raspberry Pi GPIO_OUT(LED)
#define GPIO_IN 11  //PIR sensor

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VAJID");
MODULE_DESCRIPTION("A Simple GPIO Driver for Raspberry Pi");
MODULE_VERSION("1.0");

static int gpio_open(struct inode *inode, struct file *file) {
    pr_info("GPIO Device opened\n");
    return 0;
}

static int gpio_close(struct inode *inode, struct file *file) {
    pr_info("GPIO Device closed\n");
    return 0;
}

static ssize_t gpio_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int value = gpio_get_value(GPIO_IN);  // Read GPIO pin value
    char buffer[2];
    snprintf(buffer, 2, "%d", value);

    if (copy_to_user(buf, buffer, 2)) {
        pr_err("Failed to send data to user\n");
        return -EFAULT;
    }

    pr_info("GPIO Device read: %d\n", value);
    return 2;  // Number of bytes sent
}

static ssize_t gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char buffer[2];

    if (copy_from_user(buffer, buf, len)) {
        pr_err("Failed to receive data from user\n");
        return -EFAULT;
    }

    buffer[len] = '\0';
    int value = simple_strtol(buffer, NULL, 10);

    if (value == 0 || value == 1) {
        gpio_set_value(GPIO_OUT, value);  // Set GPIO pin value
        pr_info("GPIO Device write: %d\n", value);
    } else {
        pr_err("Invalid value: GPIO accepts 0 or 1\n");
        return -EINVAL;
    }

    return len;
}

static struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .release = gpio_close,
    .read = gpio_read,
    .write = gpio_write,
};

static int __init gpio_driver_init(void) {
    int out,in;

    // Request GPIO pin
    out = gpio_request(GPIO_OUT, "PIR_DRIVER");
    if (out) {
        pr_err("Failed to request LED as pin %d\n", GPIO_OUT);
        return ret;
    }

    // Set GPIO direction to output
    out = gpio_direction_output(GPIO_OUT, 0);
    if (out) {
        pr_err("Failed to set GPIO direction for pin %d\n", GPIO_OUT);
        gpio_free(GPIO_OUT);
        return out;
    }
    in = gpio_request(GPIO_IN , "PIR_DRIVER");
    if (in) {
        pr_err("Failed to request PIR as pin %d\n", GPIO_IN);
        return in;
    }

    // Set GPIO direction to output
    in = gpio_direction_output(GPIO_IN, 1);
    if (in) {
        pr_err("Failed to set GPIO direction for pin %d\n", GPIO_IN);
        gpio_free(GPIO_IN);
        return in;
    }


    // Register device
    out = register_chrdev(0, "gpio_driver", &gpio_fops);
    if (out < 0) {
        pr_err("Failed to register character device\n");
        gpio_free(GPIO_IN);
	gpio_free(GPIO_OUT);
        return out;
    }

    pr_info("GPIO Driver loaded\n");
    return 0;
}

static void __exit gpio_driver_exit(void) {
    unregister_chrdev(240, "gpio_driver");
    gpio_free(GPIO_IN);
    gpio_free(GPIO_OUT);
    pr_info("GPIO Driver unloaded\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);
