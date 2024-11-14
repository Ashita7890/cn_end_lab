// child.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char buffer[100];

    // Read from standard input (which is redirected to the pipe in parent.c)
    if (read(STDIN_FILENO, buffer, sizeof(buffer)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("Child received message: %s\n", buffer);

    return 0;
}
