#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define NUM_FDS 3

void recv_fds(int socket, int *fds, int num_fds) {
    struct msghdr msg = {0};
    struct iovec iov[1];
    char buffer[1];
    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // Control message buffer to receive file descriptors
    char control_buf[CMSG_SPACE(num_fds * sizeof(int))];
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);

    // Receive the message containing the file descriptors
    if (recvmsg(socket, &msg, 0) == -1) {
        perror("recvmsg failed");
        exit(EXIT_FAILURE);
    }

    // Extract the file descriptors from the control message
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg == NULL || cmsg->cmsg_len != CMSG_LEN(num_fds * sizeof(int))) {
        fprintf(stderr, "Invalid control message\n");
        exit(EXIT_FAILURE);
    }
    if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
        fprintf(stderr, "Unexpected control message level/type\n");
        exit(EXIT_FAILURE);
    }

    memcpy(fds, CMSG_DATA(cmsg), num_fds * sizeof(int));
}

int main() {
    int client_fd;
    struct sockaddr_un addr;
    int fds[NUM_FDS];

    // Create the Unix Domain Socket
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

    // Receive multiple file descriptors from the server
    recv_fds(client_fd, fds, NUM_FDS);

    // Test the received file descriptors
    for (int i = 0; i < NUM_FDS; i++) {
        char buffer[128];
        ssize_t bytes_read = read(fds[i], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Data from file descriptor %d: %s\n", fds[i], buffer);
        } else if (bytes_read == 0) {
            printf("EOF on file descriptor %d\n", fds[i]);
        } else {
            perror("Read failed");
        }
        close(fds[i]);  // Close the received file descriptor
    }

    // Clean up and close the client socket
    close(client_fd);

    return 0;
}
