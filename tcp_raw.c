#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); // Change to ICMP or other protocol
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    char buffer[4096];
    printf("Bol bhai...\n");
    while (1) {
        memset(buffer, 0, 4096);

        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);

        ssize_t dataSize = recvfrom(sock, buffer, 4096, 0, (struct sockaddr *)&addr, &addr_len);
        if (dataSize < 0) {
            perror("Receive failed");
            return 1;
        }

        printf("Received Packet Size: %zd bytes\n", dataSize);

        // Extract the IP header
        struct ip* iph = (struct ip*)buffer;
        int iphLen = iph->ip_hl * 8;  // Header length in bytes

        // Print source and destination IP addresses
        printf("Source IP: %s\n", inet_ntoa(iph->ip_src));
        printf("Destination IP: %s\n", inet_ntoa(iph->ip_dst));

        // Extract and print the payload (skip the IP header)
        char* payload = buffer + iphLen;
        printf("Payload: %s\n", payload);
    }

    close(sock);
    return 0;
}