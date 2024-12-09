#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define UART_DEVICE "/dev/ttyUSB0" // Replace with your UART device file

int main() {
    int uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Unable to open UART");
        return -1;
    }

    // Configure UART
    struct termios options;
    tcgetattr(uart_fd, &options);       // Get current UART settings
    cfsetispeed(&options, B9600);      // Set input baud rate
    cfsetospeed(&options, B9600);      // Set output baud rate
    options.c_cflag |= (CLOCAL | CREAD); // Enable receiver and set local mode
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;            // 8 data bits
    options.c_cflag &= ~PARENB;        // No parity
    options.c_cflag &= ~CSTOPB;        // 1 stop bit
    options.c_cflag &= ~CRTSCTS;       // No hardware flow control
    tcsetattr(uart_fd, TCSANOW, &options); // Apply the settings

    printf("UART Loopback Test\n");

    // Test message
    const char *test_message = "Hello, Loopback!\n";
    char read_buffer[256];
    memset(read_buffer, 0, sizeof(read_buffer));

    // Send data
    int bytes_written = write(uart_fd, test_message, strlen(test_message));
    if (bytes_written < 0) {
        perror("UART write failed");
        close(uart_fd);
        return -1;
    }
    printf("Sent: %s", test_message);

    // Read the echoed data
    usleep(100000); // Small delay to allow data to loop back
    int bytes_read = read(uart_fd, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read < 0) {
        perror("UART read failed");
    } else {
        printf("Received: %s", read_buffer);
    }

    close(uart_fd);
    return 0;
}
