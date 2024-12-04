#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/simple_device"

int main() {
    char buffer[1024];
    int fd = open(DEVICE_PATH, O_RDONLY);

    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read < 0) {
        perror("Failed to read from device");
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the string
    printf("Data from driver: %s\n", buffer);

    close(fd);
    return 0;
}
