// parent.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
    int pipefd[2];
    pid_t pid;
    char message[] = "Hello from parent process!";

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        // Close the write-end of the pipe, as child only reads
        close(pipefd[1]);

        // Duplicate the read end of the pipe to standard input (fd 0)
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Execute the child program (receiver)
        execl("./child", "child", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        // Close the read-end of the pipe, as parent only writes
        close(pipefd[0]);

        // Write a message to the pipe
        write(pipefd[1], message, strlen(message) + 1);

        // Close the write-end of the pipe
        close(pipefd[1]);
    }

    return 0;
}
/*The parent process creates a pipe and forks a child process.
The child process executes a separate program using exec(), which reads data from the pipe.
The parent writes a message to the pipe, and the child reads it.*/