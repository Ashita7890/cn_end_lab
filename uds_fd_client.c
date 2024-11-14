#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket"

int recv_fd(int socket) {
    struct msghdr msg = {0};
    struct iovec iov[1];
    char buffer[1];
    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // Prepare to receive the control message that contains the file descriptor
    char control_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);

    if (recvmsg(socket, &msg, 0) == -1) {
        perror("recvmsg failed");
        exit(EXIT_FAILURE);
    }

    // Extract the file descriptor from the control message
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg == NULL || cmsg->cmsg_len != CMSG_LEN(sizeof(int))) {
        fprintf(stderr, "Invalid control message\n");
        exit(EXIT_FAILURE);
    }
    if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
        fprintf(stderr, "Unexpected control message level/type\n");
        exit(EXIT_FAILURE);
    }

    int fd_received = *((int *) CMSG_DATA(cmsg));
    return fd_received;
}

int main() {
    int client_fd;
    struct sockaddr_un addr;

    // Create a Unix Domain Socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the address for the UDS
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Receive the file descriptor from the server
    int received_fd = recv_fd(client_fd);
    printf("Received file descriptor: %d\n", received_fd);

    // Optionally read from the received file descriptor to test it
    char buffer[128];
    ssize_t bytes_read = read(received_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Data from received file descriptor: %s\n", buffer);
    } else if (bytes_read == 0) {
        printf("EOF on received file descriptor\n");
    } else {
        perror("Read from received file descriptor failed");
    }

    // Clean up and close the client socket
    close(client_fd);

    return 0;
}
