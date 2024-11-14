#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

// Thread function to handle each client
void* handle_client(void* client_socket) {
    int client_fd = *(int*)client_socket;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Receive and send messages to the client
    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the buffer
        printf("Client %d: %s\n", client_fd, buffer);

        // Echo the message back to the client
        send(client_fd, buffer, bytes_received, 0);
    }

    printf("Client %d disconnected.\n", client_fd);
    close(client_fd);
    free(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a client connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Connected to client with fd %d\n", client_fd);

        // Allocate memory for the client socket descriptor
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_fd;

        // Create a new thread for each client connection
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)new_sock) != 0) {
            perror("Failed to create thread");
            free(new_sock);
            continue;
        }

        pthread_detach(thread_id);  // Detach thread to free resources automatically when it ends
    }

    close(server_fd);
    return 0;
}
