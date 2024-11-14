#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/uds_socket"

void send_fd(int socket, int fd_to_send) {
    struct msghdr msg = {0};
    struct iovec iov[1];
    char buffer[1] = {'F'};  // Dummy byte to send with control message
    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);
    
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // Set up the control message to send the file descriptor
    char control_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *) CMSG_DATA(cmsg)) = fd_to_send;

    if (sendmsg(socket, &msg, 0) == -1) {
        perror("sendmsg failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[128];

    // Create the Unix Domain Socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Remove the old socket file if it exists
    unlink(SOCKET_PATH);

    // Configure the address for the UDS
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    // Accept a connection from a client
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Send the server_fd file descriptor to the client
    send_fd(client_fd, server_fd);

    printf("Sent file descriptor to the client\n");

    // Clean up and close the server socket
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
