#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define NUM_FDS 3

void send_fds(int socket, int *fds, int num_fds) {
    struct msghdr msg = {0};
    struct iovec iov[1];
    char buffer[1] = {0};  // A placeholder byte to send with the control message
    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // Control message to pass the file descriptors
    char control_buf[CMSG_SPACE(num_fds * sizeof(int))];
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(num_fds * sizeof(int));

    // Copy the file descriptors into the control message
    memcpy(CMSG_DATA(cmsg), fds, num_fds * sizeof(int));

    // Send the message containing the file descriptors
    if (sendmsg(socket, &msg, 0) == -1) {
        perror("sendmsg failed");
        exit(EXIT_FAILURE);
    }

    printf("Sent %d file descriptors to the client\n", num_fds);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    int fds[NUM_FDS];

    // Open files to pass their descriptors (just for demonstration)
    for (int i = 0; i < NUM_FDS; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "/tmp/file%d.txt", i + 1);
        fds[i] = open(filename, O_CREAT | O_RDWR, 0666);
        if (fds[i] == -1) {
            perror("open failed");
            exit(EXIT_FAILURE);
        }
        write(fds[i], "Hello from file\n", 16);  // Write some data to the file
    }

    // Create the Unix Domain Socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Remove any existing socket file
    unlink(SOCKET_PATH);

    // Configure the address for the UDS
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    // Bind and listen for incoming connections
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    // Accept a connection from a client
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Send multiple file descriptors to the client
    send_fds(client_fd, fds, NUM_FDS);

    // Clean up and close
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    // Close the file descriptors
    for (int i = 0; i < NUM_FDS; i++) {
        close(fds[i]);
    }

    return 0;
}
