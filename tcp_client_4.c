
// #include <arpa/inet.h>
// #include <stdio.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <unistd.h>
// #define PORT 8081

// int main(int argc, char const* argv[]) {
//     int client_fd;
//     struct sockaddr_in serv_addr;
//     char* hello = "Hello from client";
//     char buffer[1024] = { 0 };
    
//     if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         printf("\nSocket creation error\n");
//         return -1;
//     }

//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(PORT);

//     // Convert IPv4 address from text to binary
//     if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
//         printf("\nInvalid address/Address not supported\n");
//         return -1;
//     }

//     if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
//         printf("\nConnection Failed\n");
//         return -1;
//     }
//     char msg[100];
//     fgets(msg,sizeof(msg),stdin);
//     send(client_fd, msg, sizeof(msg), 0);
//     printf("Hello message sent\n");
    
//     read(client_fd, buffer, 1024);
//     printf("%s\n", buffer);

//     // Close the socket
//     close(client_fd);
//     return 0;
// }

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8081

int main() {
    int client_fd;
    struct sockaddr_in serv_addr;
    char *message = "Hello from client";
    char buffer[1024] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
    
    send(client_fd, message, strlen(message), 0);  // Send message to server
    printf("Message sent: %s\n", message);

    read(client_fd, buffer, sizeof(buffer));  // Receive response
    printf("Server reply: %s\n", buffer);

    close(client_fd);  // Close socket
    return 0;
}
