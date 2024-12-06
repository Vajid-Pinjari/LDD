#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

#define PIR_OUT 17  // Pin number on Raspberry Pi GPIO_OUT(LED)
#define PIR_IN 11  //PIR sensor

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VAJID");
MODULE_DESCRIPTION("PIR Driver for Raspberry Pi");
MODULE_VERSION("1.0");

static int pir_open(struct inode *inode, struct file *file) {
    pr_info("PIR Device opened\n");
    return 0;
}

static int pir_close(struct inode *inode, struct file *file) {
    pr_info("PIR Device closed\n");
    return 0;
}

static ssize_t pir_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int value = gpio_get_value(PIR_IN);  // Read from PIR pin
    char buffer[2];
    snprintf(buffer, 2, "%d", value);

    if (copy_to_user(buf, buffer, 2)) {
        pr_err("Failed to send data to user\n");
        return -EFAULT;
    }

    pr_info("PIR Device read: %d\n", value);
    return 2;  // Number of bytes sent
}

static ssize_t pir_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char buffer[2];

    if (copy_from_user(buffer, buf, len)) {
        pr_err("Failed to receive data from user\n");
        return -EFAULT;
    }

    buffer[len] = '\0';
    int value = simple_strtol(buffer, NULL, 10);

    if (value == 0 || value == 1) {
        gpio_set_value(PIR_OUT, value);  // Set GPIO pin value
        pr_info("GPIO Device write: %d\n", value);
    } else {
        pr_err("Invalid value: PIR accepts 0 or 1\n");
        return -EINVAL;
    }

    return len;
}

static struct file_operations pir_fops = {
    .owner = THIS_MODULE,
    .open = pir_open,
    .release = pir_close,
    .read = pir_read,
    .write = pir_write,
};

static int __init pir_driver_init(void) {
    int out,in;

    // Request GPIO pin
    out = gpio_request(PIR_OUT, "PIR_DRIVER");
    if (out) {
        pr_err("Failed to request LED as pin %d\n", PIR_OUT);
        return out;
    }

    // Set GPIO direction to output
    out = gpio_direction_output(PIR_OUT, 0);
    if (out) {
        pr_err("Failed to set GPIO direction for pin %d\n", PIR_OUT);
        gpio_free(PIR_OUT);
        return out;
    }
    in = gpio_request(PIR_IN , "PIR_DRIVER");
    if (in) {
        pr_err("Failed to request PIR as pin %d\n",PIR_IN);
        return in;
    }

    // Set GPIO direction to output
    in = gpio_direction_output(PIR_IN, 1);
    if (in) {
        pr_err("Failed to set GPIO direction for pin %d\n", PIR_IN);
        gpio_free(PIR_IN);
        return in;
    }


    // Register device
    out = register_chrdev(0, "pir_driver", &pir_fops);
    if (out < 0) {
        pr_err("Failed to register pir driver\n");
        gpio_free(PIR_IN);
	gpio_free(PIR_OUT);
        return out;
    }

    pr_info("PIR Driver loaded\n");
    return 0;
}

static void __exit pir_driver_exit(void) {
    unregister_chrdev(240, "pir_driver");
    gpio_free(PIR_IN);
    gpio_free(PIR_OUT);
    pr_info("PIR Driver unloaded\n");
}

module_init(pir_driver_init);
module_exit(pir_driver_exit);
