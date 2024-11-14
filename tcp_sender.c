#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>  // For the IP header
#include <netinet/ip_icmp.h> // For the ICMP header
#include <unistd.h>

// Checksum calculation
unsigned short checksum(void *b, int len) 
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        perror("Error in creation");
        return 1;
    }

    char buffer[4096];
    memset(buffer, 0, 4096);

    struct ip* iph = (struct ip*)buffer;
    char* data = buffer + sizeof(struct ip);

    strcpy(data, "Hello from sender");

    iph->ip_hl = 5;  // Header length
    iph->ip_v = 4;   // IP version
    iph->ip_off = 0;
    iph->ip_tos = 0;
    iph->ip_len = sizeof(struct ip) + strlen(data); // Total packet length
    iph->ip_id = htons(54321);
    iph->ip_sum = 0;
    iph->ip_ttl = 64;  // Time to live
    iph->ip_src.s_addr = inet_addr("192.2.2.1");
    iph->ip_dst.s_addr = inet_addr("127.0.0.1");
    iph->ip_p = IPPROTO_RAW;  

    iph->ip_sum = checksum((unsigned short *)buffer, sizeof(struct ip));

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(0); // No port for raw socket
    dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // Correct destination

    if (sendto(sock, buffer, iph->ip_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("Send failed");
        return 1;
    }

    printf("Packet sent successfully!\n");
    close(sock);
    return 0;
}