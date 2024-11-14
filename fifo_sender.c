// sender.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 


int main() 
{
    int fifo_fd;
    char message[] = "Hello from sender process!";
    const char *FIFO_PATH  = "/tmp/my_fifo";
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
    }

    // Open the FIFO for writing
    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write the message to the FIFO
    write(fifo_fd, message, strlen(message) + 1);

    // Close the FIFO
    close(fifo_fd);

    printf("Message sent to FIFO: %s\n", message);

    return 0;
}
