#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/gpio_ioctl"

// ioctl command definitions (must match driver)
#define GPIO_MAGIC     'G'
#define GPIO_SET_DIR   _IOW(GPIO_MAGIC, 0, int)
#define GPIO_WRITE     _IOW(GPIO_MAGIC, 1, int)
#define GPIO_READ      _IOR(GPIO_MAGIC, 2, int)

int main()
{
    int fd, value;

    // Open the device
    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    // Set GPIO direction to output
    if (ioctl(fd, GPIO_SET_DIR, 1) < 0) {
        perror("Failed to set GPIO direction");
        close(fd);
        return -1;
    }

    // Write a value to the GPIO
    value = 1;
    if (ioctl(fd, GPIO_WRITE, &value) < 0) {
        perror("Failed to write to GPIO");
        close(fd);
        return -1;
    }
    printf("Wrote %d to GPIO\n", value);

    // Set GPIO direction to input
    if (ioctl(fd, GPIO_SET_DIR, 0) < 0) {
        perror("Failed to set GPIO direction");
        close(fd);
        return -1;
    }

    // Read a value from the GPIO
    if (ioctl(fd, GPIO_READ, &value) < 0) {
        perror("Failed to read from GPIO");
        close(fd);
        return -1;
    }
    printf("Read %d from GPIO\n", value);

    // Close the device
    close(fd);
    return 0;
}
