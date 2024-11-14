#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SUPER_PORT 8080
#define SUBSERVER1_IP "127.0.0.1"
#define SUBSERVER1_PORT 8081
#define SUBSERVER2_IP "127.0.0.1"
#define SUBSERVER2_PORT 8082

void forward_to_subserver(const char *subserver_ip, int subserver_port, const char *message) {
    int subserver_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (subserver_fd == -1) 
    {
        perror("Subserver socket creation failed");
        return;
    }

    struct sockaddr_in subserver_addr;
    subserver_addr.sin_family = AF_INET;
    subserver_addr.sin_port = htons(subserver_port);
    inet_pton(AF_INET, subserver_ip, &subserver_addr.sin_addr);

    if (connect(subserver_fd, (struct sockaddr *)&subserver_addr, sizeof(subserver_addr)) == -1) {
        perror("Failed to connect to subserver");
        close(subserver_fd);
        return;
    }

    // Send the message to the subserver
    send(subserver_fd, message, strlen(message), 0);

    // (Optional) Receive response from the subserver
    char buffer[256];
    ssize_t bytes_received = recv(subserver_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Response from subserver: %s\n", buffer);
    }
    int pipefd[2];
    pid_t pid;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
       
        close(pipefd[1]);

        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Execute the child program (receiver)
        execl("./child", "child", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        
        close(pipefd[0]);

        write(pipefd[1], buffer, sizeof(buffer));

        close(pipefd[1]);
    }

    close(subserver_fd);
}

int main() {
    int super_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (super_fd == -1) 
    {
        perror("Superserver socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in super_addr;
    super_addr.sin_family = AF_INET;
    super_addr.sin_addr.s_addr = INADDR_ANY;
    super_addr.sin_port = htons(SUPER_PORT);

    if (bind(super_fd, (struct sockaddr *)&super_addr, sizeof(super_addr)) == -1) {
        perror("Bind failed");
        close(super_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(super_fd, 5) == -1) {
        perror("Listen failed");
        close(super_fd);
        exit(EXIT_FAILURE);
    }

    printf("Superserver is listening on port %d\n", SUPER_PORT);

    int client_count = 0;

    while (1) {
        int client_fd = accept(super_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }

        char buffer[256];
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            close(client_fd);
            continue;
        }
        buffer[bytes_read] = '\0';

        printf("Received message from client: %s\n", buffer);

        // Choose subserver based on round-robin
        if (client_count % 2 == 0) {
            forward_to_subserver(SUBSERVER1_IP, SUBSERVER1_PORT, buffer);
        } else {
            forward_to_subserver(SUBSERVER2_IP, SUBSERVER2_PORT, buffer);
        }
        client_count++;

        close(client_fd);
    }

    close(super_fd);
    return 0;
}