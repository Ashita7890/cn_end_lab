// #include <stdio.h>
// #include <fcntl.h>
// #include <sys/wait.h>
// #include <sys/types.h>
// #include <sys/socket.h>   
// #include <netinet/in.h>   
// #include <arpa/inet.h>    
// #include <unistd.h>       
// // #include <stdlib.h>
// #include <poll.h>
// #include <string.h>

// #define NUM_PORTS 5

// int ports[NUM_PORTS] = {8081, 8082, 8083, 8084, 8085};
// char *args[][2] = {{"./S1", NULL}, {"./S2", NULL}, {"./S3", NULL}, {"./S4", NULL}, {"./S5", NULL}};

// void fork_and_exec1(int nsfd, int i) {
//     pid_t p = fork();
//     if (p > 0) {
//         close(nsfd);  // Parent closes the connected socket
//     } else if (p == 0) {
//         // Close all file descriptors except the socket for the child process
//         close(nsfd);

//         // Redirect nsfd to stdin and stdout for the child process (S3)
//         dup2(nsfd, STDIN_FILENO);  // Redirect nsfd to stdin
//         dup2(nsfd, STDOUT_FILENO); // Redirect nsfd to stdout

//         // Execute the S3 server process
//         execvp("./S3", (char*[]){"./S3", NULL});
//         perror("execvp failed");
//         exit(EXIT_FAILURE);
//     }
// }


// int main() {
//     char BUFFER[1024];
//     int sfd[NUM_PORTS];
//     struct sockaddr_in address[NUM_PORTS];

//     for (int i = 0; i < NUM_PORTS; ++i) {
//         if (i % 2 == 0) {  // TCP socket
//             if ((sfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//                 perror("TCP socket failed");
//                 exit(EXIT_FAILURE);
//             }
//             int opt = 1;
//             if (setsockopt(sfd[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//                 perror("setsockopt failed");
//                 exit(EXIT_FAILURE);
//             }
//         } else {  // UDP socket
//             if ((sfd[i] = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//                 perror("UDP socket failed");
//                 exit(EXIT_FAILURE);
//             }
//         }

//         address[i].sin_family = AF_INET;
//         address[i].sin_addr.s_addr = INADDR_ANY;
//         address[i].sin_port = htons(ports[i]);

//         if (bind(sfd[i], (struct sockaddr*)&address[i], sizeof(address[i])) < 0) {
//             perror("bind failed");
//             exit(EXIT_FAILURE);
//         }

//         if (i % 2 == 0) {  // TCP sockets need to listen
//             if (listen(sfd[i], 5) < 0) {
//                 perror("listen failed");
//                 exit(EXIT_FAILURE);
//             }
//         }
//     }

//     struct pollfd pfd[NUM_PORTS];
//     for (int i = 0; i < NUM_PORTS; ++i) {
//         pfd[i].fd = sfd[i];
//         pfd[i].events = POLLIN;
//     }

//     while (1) {
//         int ret = poll(pfd, NUM_PORTS, 5000);
//         if (ret > 0) {
//             for (int i = 0; i < NUM_PORTS; ++i) {
//                 if (pfd[i].revents & POLLIN) {
//                     if (i % 2 == 0) {  // TCP
//                         socklen_t addrlen = sizeof(address[i]);
//                         int nsfd = accept(sfd[i], (struct sockaddr*)&address[i], &addrlen);
//                         if (nsfd < 0) {
//                             perror("accept failed");
//                             continue;
//                         }
//                         fork_and_exec1(nsfd, i);  // Handle the new connection
//                     } else {  // UDP
//                         socklen_t addrlen = sizeof(address[i]);
//                         ssize_t sz = recvfrom(sfd[i], BUFFER, sizeof(BUFFER), 0, (struct sockaddr*)&address[i], &addrlen);
//                         if (sz < 0) {
//                             perror("recvfrom failed");
//                         } else {
//                             BUFFER[sz] = '\0';
//                             printf("Received on UDP port %d: %s\n", ports[i], BUFFER);
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     for (int i = 0; i < NUM_PORTS; ++i) {
//         close(sfd[i]);
//     }
    
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define NUM_PORTS 1
int ports[NUM_PORTS] = {8081};  // Just one port for simplicity

void fork_and_exec(int nsfd) {
    pid_t p = fork();
    if (p > 0) {
        close(nsfd);  // Parent closes the connected socket
    } else if (p == 0) {
        // // Redirect nsfd to stdin and stdout for the child process
        // dup2(nsfd, STDIN_FILENO);  // Redirect nsfd to stdin
        // dup2(nsfd, STDOUT_FILENO); // Redirect nsfd to stdout
        // close(nsfd);               // Close nsfd after duplicating it

        // // Execute the S3 server process
        // execlp("./S3", "S3", NULL);  // Running S3 as a new process
        // perror("execlp failed");
        // exit(EXIT_FAILURE);

        char nsfd_str[20];
        sprintf(nsfd_str, "%d", nsfd);  // Convert socket descriptor to string

        // Pass the socket descriptor to the S3 process as an argument
        execlp("./S3", "S3", nsfd_str, NULL);  // Running S3 as a new process
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sfd;
    struct sockaddr_in address;

    // Create a TCP socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(ports[0]);

    // Bind the socket to the address and port
    if (bind(sfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sfd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    struct pollfd pfd = { .fd = sfd, .events = POLLIN };
    while (1) {
        int ret = poll(&pfd, 1, 5000);  // Poll for events
        if (ret > 0) {
            if (pfd.revents & POLLIN) {
                int nsfd = accept(sfd, NULL, NULL);  // Accept incoming connection
                if (nsfd < 0) {
                    perror("accept failed");
                    continue;
                }
                printf("Accepted connection\n");
                fork_and_exec(nsfd);  // Handle the connection by forking
            }
        }
    }

    close(sfd);
    return 0;
}

