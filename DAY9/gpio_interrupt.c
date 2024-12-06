/*************************//** 
 *  \file       gpio_interrupt.c
 *
 *  \details    Basic GPIO interrupt driver for Raspberry Pi 4
 *
 *  \author     Vajid
 *
 ***************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>     // GPIO functions
#include <linux/interrupt.h> // Interrupts

#define GPIO_BUTTON  514  // GPIO pin for the button
#define GPIO_LED     515  // GPIO pin for the LED

static int irq_number;       // IRQ number for the button GPIO
static dev_t dev = 0;        // Device number
static struct class *dev_class;
static struct cdev gpio_cdev;

// Prototypes
static irqreturn_t button_irq_handler(int irq, void *dev_id);
static int __init gpio_driver_init(void);
static void __exit gpio_driver_exit(void);

/*
** IRQ handler for button press
*/
static irqreturn_t button_irq_handler(int irq, void *dev_id)
{
    static bool led_state = false;
    led_state = !led_state;  // Toggle LED state
    gpio_set_value(GPIO_LED, led_state);
    pr_info("Button pressed: LED toggled to %d\n", led_state);
    return IRQ_HANDLED;
}

/*
** Module init function
*/
static int __init gpio_driver_init(void)
{
    // Allocate device number
    if ((alloc_chrdev_region(&dev, 0, 1, "gpio_dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    // Create cdev structure
    cdev_init(&gpio_cdev, NULL);

    // Add character device to the system
    if ((cdev_add(&gpio_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_unregister;
    }

    // Create struct class
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "gpio_class"))) {
        pr_err("Cannot create the struct class\n");
        goto r_cdev_del;
    }

    // Create device
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "gpio_device"))) {
        pr_err("Cannot create the Device\n");
        goto r_class_destroy;
    }

    // Request GPIO for button
    if (!gpio_is_valid(GPIO_BUTTON)) {
        pr_err("Invalid GPIO: %d\n", GPIO_BUTTON);
        goto r_device_destroy;
    }
    if (gpio_request(GPIO_BUTTON, "gpio_button") < 0) {
        pr_err("Failed to request GPIO: %d\n", GPIO_BUTTON);
        goto r_device_destroy;
    }

    // Configure button GPIO as input with pull-up
    gpio_direction_input(GPIO_BUTTON);
    gpiod_set_debounce(GPIO_BUTTON, 200); // Debounce time in ms

    // Request GPIO for LED
    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("Invalid GPIO: %d\n", GPIO_LED);
        goto r_gpio_free_button;
    }
    if (gpio_request(GPIO_LED, "gpio_led") < 0) {
        pr_err("Failed to request GPIO: %d\n", GPIO_LED);
        goto r_gpio_free_button;
    }

    // Configure LED GPIO as output
    gpio_direction_output(GPIO_LED, 0);

    // Get IRQ number for the button GPIO
    irq_number = gpio_to_irq(GPIO_BUTTON);
    if (irq_number < 0) {
        pr_err("Cannot get IRQ number for GPIO %d\n", GPIO_BUTTON);
        goto r_gpio_free_led;
    }

    // Request IRQ for the button GPIO
    if (request_irq(irq_number, button_irq_handler, IRQF_TRIGGER_FALLING, "gpio_button_irq", NULL) < 0) {
        pr_err("Failed to request IRQ\n");
        goto r_gpio_free_led;
    }

    pr_info("GPIO Interrupt Driver Loaded\n");
    return 0;

r_gpio_free_led:
    gpio_free(GPIO_LED);
r_gpio_free_button:
    gpio_free(GPIO_BUTTON);
r_device_destroy:
    device_destroy(dev_class, dev);
r_class_destroy:
    class_destroy(dev_class);
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
    free_irq(irq_number, NULL);
    gpio_set_value(GPIO_LED, 0); // Turn off LED
    gpio_free(GPIO_LED);
    gpio_free(GPIO_BUTTON);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("GPIO Interrupt Driver Removed\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vajid");
MODULE_DESCRIPTION("Basic GPIO Interrupt Driver for Raspberry Pi 4");
MODULE_VERSION("1.0");
