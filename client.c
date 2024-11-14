#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SUPER_IP "127.0.0.1"  // Superserver IP address
#define SUPER_PORT 8080       // Superserver port

int main() {
    int client_fd;
    struct sockaddr_in super_addr;
    char message[256];
    char buffer[256];

    // Create a socket for connecting to the superserver
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define the superserver's address
    super_addr.sin_family = AF_INET;
    super_addr.sin_port = htons(SUPER_PORT);
    if (inet_pton(AF_INET, SUPER_IP, &super_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Connect to the superserver
    if (connect(client_fd, (struct sockaddr *)&super_addr, sizeof(super_addr)) == -1) {
        perror("Connection to superserver failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to superserver at %s:%d\n", SUPER_IP, SUPER_PORT);

    // Send a message to the superserver
    printf("Enter message to send to the superserver: ");
    fgets(message, sizeof(message), stdin);
    send(client_fd, message, strlen(message), 0);

    // Receive a response from the superserver (or subserver)
    // ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    // if (bytes_received > 0) {
    //     buffer[bytes_received] = '\0';
    //     printf("Response from server: %s\n", buffer);
    // } else {
    //     printf("No response from server.\n");
    // }
    
    // char buffer[100];

    // Read from standard input (which is redirected to the pipe in parent.c)
    
    if (read(STDIN_FILENO, buffer, sizeof(buffer)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("client received message: %s\n", buffer);
    // Close the client socket
    close(client_fd);
    return 0;
}
