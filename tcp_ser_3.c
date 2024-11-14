#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Function to handle client communication
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Read data from the client
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the string
        printf("Received from client: %s\n", buffer);

        // Send acknowledgment back to the client
        write(client_socket, "Message received", strlen("Message received"));
    }

    if (bytes_read == 0) {
        printf("Client disconnected\n");
    } else if (bytes_read == -1) {
        perror("read failed");
    }

    // Close the client socket
    close(client_socket);
}

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in address;
    struct pollfd fds[MAX_CLIENTS + 1]; // We have the server fd + clients
    int nfds = 1; // Number of file descriptors being monitored
    int addrlen = sizeof(address);
    int i, poll_result;

    // Initialize client_sockets array
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Create the server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Setup the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the server socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Add server socket to the fds array for polling
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Poll for any activity on server socket or client sockets
        poll_result = poll(fds, nfds, -1); // Infinite timeout

        if (poll_result == -1) {
            perror("Poll error");
            continue;
        }

        // Check if there's an incoming connection on the server socket
        if (fds[0].revents & POLLIN) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            if (new_socket < 0) {
                perror("Accept failed");
                continue;
            }

            printf("New connection established\n");

            // Find an empty slot for the new client
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    fds[nfds].fd = new_socket;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    break;
                }
            }

            if (i == MAX_CLIENTS) {
                printf("Max clients reached. Rejecting new connection.\n");
                close(new_socket); // Reject the client
            }
        }

        // Check for activity on each of the client sockets
        for (i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                // Fork a child process to handle the client
                pid_t pid = fork();
                if (pid == 0) {  // Child process
                    close(server_fd); // Close the server socket in the child process
                    handle_client(fds[i].fd);
                    exit(0); // Exit child process after handling the client
                } else if (pid < 0) {
                    perror("Fork failed");
                } else {
                    // Parent process: Close the client socket in the parent and continue
                    close(fds[i].fd);
                    client_sockets[i - 1] = 0; // Mark the client socket as free
                    fds[i].fd = -1; // Reset the fd in the pollfd array
                    nfds--;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
