#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>   // Provides declarations for the IP header
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

// Function to print the IP header
void print_ip_header(unsigned char *buffer) {
    struct ip *iph = (struct ip *)buffer;
    struct sockaddr_in src, dest;

    // Get source and destination IP addresses
    src.sin_addr.s_addr = iph->ip_src.s_addr;
    dest.sin_addr.s_addr = iph->ip_dst.s_addr;

    printf("\n\nIP Header\n");
    printf("   |-IP Version        : %d\n", (unsigned int)iph->ip_v);
    printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n", (unsigned int)iph->ip_hl, ((unsigned int)(iph->ip_hl)) * 4);
    printf("   |-Type of Service   : %d\n", (unsigned int)iph->ip_tos);
    printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n", ntohs(iph->ip_len));
    printf("   |-Identification    : %d\n", ntohs(iph->ip_id));
    printf("   |-TTL               : %d\n", (unsigned int)iph->ip_ttl);
    printf("   |-Protocol          : %d\n", (unsigned int)iph->ip_p);
    printf("   |-Checksum          : %d\n", ntohs(iph->ip_sum));
    printf("   |-Source IP         : %s\n", inet_ntoa(src.sin_addr));
    printf("   |-Destination IP    : %s\n", inet_ntoa(dest.sin_addr));
}

int main() {
    int sock;
    struct sockaddr_in addr;
    socklen_t addr_len;
    unsigned char buffer[65536];

    // Create a raw socket to receive all packets at the IP level (on macOS use IPPROTO_IP)
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);  // IPPROTO_IP captures all IP packets
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Starting packet capture...\n");

    // Loop to capture and process packets
    while (1) {
        addr_len = sizeof(addr);
        // Receive packets
        int packet_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
        if (packet_size < 0) {
            perror("Failed to receive packets");
            return 1;
        }

        // Print the IP header information
        print_ip_header(buffer);
    }

    close(sock);
    return 0;
}
