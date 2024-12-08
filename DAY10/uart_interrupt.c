#include <linux/module.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#define DRIVER_NAME "rpi_uart"
#define UART_BASE   0xfe201000  // Base address of UART (check your RPi datasheet)
#define UART_IRQ    57          // UART interrupt number for RPi 4B (adjust if needed)

#define UART_FIFO_SIZE 16       // FIFO size of PL011 UART

static void __iomem *uart_base; // Mapped UART base address

// UART read/write helpers
static inline void uart_write(u32 value, u32 offset)
{
    writel(value, uart_base + offset);
}

static inline u32 uart_read(u32 offset)
{
    return readl(uart_base + offset);
}

// Interrupt handler for UART
static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
    u32 status = uart_read(UART_REG_FR); // Flag Register

    if (!(status & UART_RXFE)) { // Data available in RX FIFO
        char ch = uart_read(UART_REG_DR); // Read received data
        pr_info("rpi_uart: Received: %c\n", ch);
    }

    if (!(status & UART_TXFF)) { // TX FIFO not full
        uart_write('A', UART_REG_DR); // Example: Send 'A'
        pr_info("rpi_uart: Transmitted: A\n");
    }

    return IRQ_HANDLED;
}

// Driver initialization
static int __init rpi_uart_init(void)
{
    int ret;

    // Map UART base address
    uart_base = ioremap(UART_BASE, 0x100);
    if (!uart_base) {
        pr_err("%s: Failed to map UART base address.\n", DRIVER_NAME);
        return -ENOMEM;
    }

    // Configure UART: Baud rate, FIFO enable
    uart_write(0x00, UART_REG_IBRD); // Integer Baud rate (adjust as needed)
    uart_write(0x00, UART_REG_FBRD); // Fractional Baud rate
    uart_write((UART_LCRH_WLEN_8 | UART_LCRH_FEN), UART_REG_LCRH); // 8-bit, FIFO enabled

    // Enable UART
    uart_write(UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE, UART_REG_CR);

    // Request IRQ
    ret = request_irq(UART_IRQ, uart_irq_handler, 0, DRIVER_NAME, NULL);
    if (ret) {
        pr_err("%s: Failed to request IRQ %d.\n", DRIVER_NAME, UART_IRQ);
        iounmap(uart_base);
        return ret;
    }

    pr_info("%s: UART driver initialized successfully.\n", DRIVER_NAME);
    return 0;
}

// Driver cleanup
static void __exit rpi_uart_exit(void)
{
    // Free IRQ
    free_irq(UART_IRQ, NULL);

    // Disable UART
    uart_write(0x00, UART_REG_CR);

    // Unmap UART base address
    iounmap(uart_base);

    pr_info("%s: UART driver removed.\n", DRIVER_NAME);
}

module_init(rpi_uart_init);
module_exit(rpi_uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba <info@techdhaba.com>");
MODULE_DESCRIPTION("UART Device Driver for Raspberry Pi 4B");
MODULE_VERSION("1.0");
