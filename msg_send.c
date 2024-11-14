// sender.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define QUEUE_KEY 12345  // Unique key for the message queue

// Define message structure
struct message {
    long mtype;            // Message type
    char mtext[100];       // Message text
};

int main() {
    int msgid;
    struct message msg;

    // Create message queue
    msgid = msgget(QUEUE_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    // Prepare message
    msg.mtype = 1;  // Message type 1
    strcpy(msg.mtext, "Hello from sender!");

    // Send message
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd failed");
        exit(1);
    }

    printf("Message sent: %s\n", msg.mtext);

    return 0;
}
