#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>   // For the IP header
#include <netinet/udp.h>  // For the UDP header
#include <unistd.h>

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

// Function to print the UDP header and data
void print_udp_header(unsigned char *buffer) {
    struct ip *iph = (struct ip *)buffer;
    unsigned short iphdrlen = iph->ip_hl * 4;

    struct udphdr *udph = (struct udphdr *)(buffer + iphdrlen);

    printf("\n\nUDP Header\n");
    printf("   |-Source Port      : %d\n", ntohs(udph->uh_sport));
    printf("   |-Destination Port : %d\n", ntohs(udph->uh_dport));
    printf("   |-Length           : %d\n", ntohs(udph->uh_ulen));
    printf("   |-Checksum         : %d\n", ntohs(udph->uh_sum));

    // Print the data part of the UDP packet
    printf("   |-Data             : %s\n", buffer + iphdrlen + sizeof(struct udphdr));
}

int main() {
    int sock;
    struct sockaddr_in addr;
    socklen_t addr_len;
    unsigned char buffer[65536]; // Larger buffer to capture complete packets

    // Create a raw socket to receive UDP packets at the IP layer
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  // Use IPPROTO_UDP to capture UDP packets
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Starting UDP packet capture...\n");

    // Loop to capture and process packets
    while (1) {
        addr_len = sizeof(addr);
        // Receive packets
        int packet_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
        if (packet_size < 0) {
            perror("Failed to receive packets");
            return 1;
        }

        struct ip *iph = (struct ip *)buffer;

        // If the protocol is UDP (protocol type 17), print the UDP header
        if (iph->ip_p == IPPROTO_UDP) {
            print_ip_header(buffer);  // Print IP header
            print_udp_header(buffer); // Print UDP header and data
        }
    }

    close(sock);
    return 0;
}
