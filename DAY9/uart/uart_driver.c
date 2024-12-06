#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#define DEVICE_NAME "uart_driver"
#define UART_PORT "/dev/ttyAMA0"

static struct file *uart_file;
//static mm_segment_t old_fs;

/* Open UART device */
static int uart_open(void)
{
    uart_file = filp_open(UART_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);
    if (IS_ERR(uart_file)) {
        pr_err("Failed to open UART device %s\n", UART_PORT);
        return PTR_ERR(uart_file);
    }

 //   old_fs = get_fs();
 //   set_fs(KERNEL_DS);
    pr_info("UART device opened successfully\n");
    return 0;
}

/* Close UART device */
static void uart_close(void)
{
    if (uart_file) {
        filp_close(uart_file, NULL);
//        set_fs(old_fs);
        pr_info("UART device closed\n");
    }
}

/* Write data to UART */
static ssize_t uart_write(const char *buf, size_t len)
{
    if (!uart_file || IS_ERR(uart_file)) {
        pr_err("UART device is not opened\n");
        return -ENODEV;
    }

    return kernel_write(uart_file, buf, len, &uart_file->f_pos);
}

/* Read data from UART */
static ssize_t uart_read(char *buf, size_t len)
{
    if (!uart_file || IS_ERR(uart_file)) {
        pr_err("UART device is not opened\n");
        return -ENODEV;
    }

    return kernel_read(uart_file, buf, len, &uart_file->f_pos);
}

/* Module Init */
static int __init uart_driver_init(void)
{
    int ret;

    pr_info("Initializing UART driver\n");
    ret = uart_open();
    if (ret)
        return ret;

    /* Example: Send a test string */
    uart_write("Hello UART\n", 11);
    return 0;
}

/* Module Exit */
static void __exit uart_driver_exit(void)
{
    uart_close();
    pr_info("Exiting UART driver\n");
}

module_init(uart_driver_init);
module_exit(uart_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Basic UART driver for Raspberry Pi");
