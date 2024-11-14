// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
// // #include <unistd.h>

// // #define MESSAGE "Hello from S3 Server\n"

// // int main() {
// //     char buffer[1024];

// //     // No need for nsfd here, since we're inheriting stdin/stdout via dup2
// //     while (1) {
// //         // Receive data from the super server (which redirected nsfd to stdin)
// //         ssize_t bytes_received = read(STDIN_FILENO, buffer, sizeof(buffer));
// //         if (bytes_received < 0) {
// //             perror("recv failed");
// //             break;
// //         } else if (bytes_received == 0) {
// //             printf("Client disconnected\n");
// //             break;
// //         }

// //         buffer[bytes_received] = '\0';  // Null-terminate the received data
// //         printf("Received from client: %s\n", buffer);

// //         // Send a response back to the client (via stdout)
// //         ssize_t bytes_sent = write(STDOUT_FILENO, MESSAGE, strlen(MESSAGE));
// //         if (bytes_sent < 0) {
// //             perror("send failed");
// //             break;
// //         }
// //     }

// //     return 0;
// // }

// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>

// #define MESSAGE "Hello from S3 Server\n"

// int main() {
//     char buffer[1024];

//     while (1) {
//         // Receive data from the super server via stdin (which is redirected from the socket)
//         ssize_t bytes_received = read(STDIN_FILENO, buffer, sizeof(buffer));
//         if (bytes_received < 0) {
//             perror("recv failed");
//             break;
//         } else if (bytes_received == 0) {
//             printf("Client disconnected\n");
//             break;
//         }

//         buffer[bytes_received] = '\0';  // Null-terminate the received data
//         printf("Received from client: %s\n", buffer);

//         // Send a response back to the client (via stdout, which is redirected to the socket)
//         ssize_t bytes_sent = write(STDOUT_FILENO, MESSAGE, strlen(MESSAGE));
//         if (bytes_sent < 0) {
//             perror("send failed");
//             break;
//         }
//     }

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define MESSAGE "Hello from S3 Server\n"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <socket_fd>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nsfd = atoi(argv[1]);  // Convert the socket descriptor from string to integer
    char buffer[1024];

    while (1) {
        // Receive data from the client via the socket
        ssize_t bytes_received = read(nsfd, buffer, sizeof(buffer));
        if (bytes_received < 0) {
            perror("recv failed");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';  // Null-terminate the received data
        printf("Received from client: %s\n", buffer);

        // Send a response back to the client via the same socket
        ssize_t bytes_sent = write(nsfd, MESSAGE, strlen(MESSAGE));
        if (bytes_sent < 0) {
            perror("send failed");
            break;
        }
    }

    close(nsfd);  // Close the socket when done
    return 0;
}
