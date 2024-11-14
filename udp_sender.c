#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>   // For the IP header
#include <netinet/udp.h>  // For the UDP header
#include <unistd.h>

// Checksum calculation
unsigned short checksum(void *b, int len) {
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
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);  // Use raw socket
    if (sock < 0) {
        perror("Error in socket creation");
        return 1;
    }

    char buffer[4096];
    memset(buffer, 0, 4096);

    // IP Header
    struct ip* iph = (struct ip*)buffer;
    // UDP Header
    struct udphdr* udph = (struct udphdr*)(buffer + sizeof(struct ip));

    // Data for UDP
    char* data = buffer + sizeof(struct ip) + sizeof(struct udphdr);
    strcpy(data, "Hello from UDP sender");

    // Fill in the IP Header
    iph->ip_hl = 5;  // Header length
    iph->ip_v = 4;   // IP version
    iph->ip_tos = 0;
    iph->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + strlen(data); // Total packet length
    iph->ip_id = htons(54321);
    iph->ip_off = 0;
    iph->ip_ttl = 64;  // Time to live
    iph->ip_src.s_addr = inet_addr("192.2.2.1"); // Source IP
    iph->ip_dst.s_addr = inet_addr("127.0.0.1"); // Destination IP
    iph->ip_p = IPPROTO_UDP; // Protocol is UDP
    iph->ip_sum = 0;  // Initially, the checksum is 0
    iph->ip_sum = checksum((unsigned short *)buffer, sizeof(struct ip)); // IP checksum

    // Fill in the UDP Header
    udph->uh_sport = htons(12345);  // Source port
    udph->uh_dport = htons(54321);  // Destination port
    udph->uh_ulen = htons(sizeof(struct udphdr) + strlen(data));  // UDP length
    udph->uh_sum = 0;  // UDP checksum (optional, can be set to 0 for no checksum)
    
    // Pseudo header for UDP checksum calculation
    struct pseudo_header {
        u_int32_t source_address;
        u_int32_t dest_address;
        u_int8_t placeholder;
        u_int8_t protocol;
        u_int16_t udp_length;
    };

    struct pseudo_header psh;

    psh.source_address = inet_addr("192.2.2.1"); // Source IP
    psh.dest_address = inet_addr("127.0.0.1"); // Destination IP
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + strlen(data));

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
    char *pseudogram = malloc(psize);

    memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + strlen(data));

    // Calculate checksum
    udph->uh_sum = checksum((unsigned short *)pseudogram, psize);

    // Set up destination address
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(0);  // No port for raw socket
    dest.sin_addr.s_addr = inet_addr("127.0.0.1"); // Correct destination

    // Send the packet
    if (sendto(sock, buffer, iph->ip_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("Send failed");
        return 1;
    }

    printf("UDP packet sent successfully!\n");

    close(sock);
    return 0;
}
