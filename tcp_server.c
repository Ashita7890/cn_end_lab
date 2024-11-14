#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Accept any incoming IP
    address.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    // Accept a connection from a client
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Connection established with client\n");

    // Read and echo the message from the client
    while (1) {
        int valread = read(new_socket, buffer, sizeof(buffer));
        if (valread <= 0) {
            printf("Client disconnected or error\n");
            break;
        }
        buffer[valread] = '\0';  // Null-terminate the string
        printf("Client: %s\n", buffer);

        // Echo the message back to the client
        send(new_socket, buffer, strlen(buffer), 0);
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
