#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");

    // Send a message to the server
    char *message = "Hello from client!";
    send(sock, message, strlen(message), 0);
    printf("Message sent to server: %s\n", message);

    // Receive a message from the server
    int valread = read(sock, buffer, sizeof(buffer));
    if (valread < 0) {
        perror("Recv failed");
        exit(EXIT_FAILURE);
    }
    buffer[valread] = '\0';  // Null-terminate the response
    printf("Server response: %s\n", buffer);

    // Close the socket
    close(sock);
    return 0;
}
