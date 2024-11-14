// receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define QUEUE_KEY 12345  // Must match sender's key

// Define message structure
struct message {
    long mtype;            // Message type
    char mtext[100];       // Message text
};

int main() {
    int msgid;
    struct message msg;

    // Access message queue
    msgid = msgget(QUEUE_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    // Receive message of type 1
    if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("msgrcv failed");
        exit(1);
    }

    printf("Message received: %s\n", msg.mtext);

    // Remove the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl failed");
        exit(1);
    }

    return 0;
}
