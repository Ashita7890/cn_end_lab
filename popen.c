#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp;

    // Open a pipe to "sort" command in write mode
    fp = popen("sort", "w");
    if (fp == NULL) {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }

    // Send data to be sorted
    fprintf(fp, "banana\n");
    fprintf(fp, "apple\n");
    fprintf(fp, "cherry\n");

    // Close the pipe
    if (pclose(fp) == -1) {
        perror("pclose failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
/*Explanation of the Write Example
Opening the Pipe for Writing:

popen("sort", "w") opens a pipe to the sort command in write mode.
Sending Data:

Using fprintf(fp, ...) sends each line to sort's standard input, which sorts the data.
Closing the Pipe:

pclose(fp) closes the pipe and retrieves the exit status of sort.

popen() is not suitable for bidirectional communication (reading and writing simultaneously) because the pipe can only be opened in one direction.*/