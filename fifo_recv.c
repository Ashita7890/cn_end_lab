// receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/my_fifo"

int main() {
    int fifo_fd;
    char buffer[100];

    // Open the FIFO for reading
    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Read the message from the FIFO
    read(fifo_fd, buffer, sizeof(buffer));

    printf("Message received from FIFO: %s\n", buffer);

    // Close the FIFO
    close(fifo_fd);

    return 0;
}
