#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>  // For the IP header
#include <sys/ioctl.h>
#include <ifaddrs.h>

#define ETH_P_IP 0x0800 // Ethernet type for IP

// Ethernet frame structure
struct ethernet_header {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short ethertype;
};

// IP Header structure
struct ip_header {
    unsigned char  iph_ihl:4, iph_ver:4;
    unsigned char  iph_tos;
    unsigned short iph_len;
    unsigned short iph_ident;
    unsigned short iph_flag:3, iph_offset:13;
    unsigned char  iph_ttl;
    unsigned char  iph_protocol;
    unsigned short iph_chksum;
    struct in_addr iph_sourceip;
    struct in_addr iph_destip;
};

// Checksum calculation for IP header
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

// Function to get the interface index
int get_interface_index(const char *interface_name) {
    struct ifaddrs *ifaddr, *ifa;
    int ifindex = -1;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            if (strcmp(ifa->ifa_name, interface_name) == 0) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ifa->ifa_addr;
                ifindex = if_nametoindex(ifa->ifa_name);
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return ifindex;
}

int main() {
    int sockfd;
    struct sockaddr_in sockaddr_in;
    struct ethernet_header eth;
    struct ip_header ip;
    char packet[4096];

    // Open raw socket (sending raw IP packets)
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Fill Ethernet header
    memset(&eth, 0, sizeof(eth));
    memset(eth.dest_mac, 0xFF, 6); // Broadcast destination MAC address
    unsigned char src_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    memcpy(eth.src_mac, src_mac, 6); // Source MAC address (your own or attacker MAC)
    eth.ethertype = htons(ETH_P_IP); // EtherType for IP protocol

    // Fill IP header
    memset(&ip, 0, sizeof(ip));

    ip.iph_ver = 4;             // IPv4
    ip.iph_ihl = 5;             // IP Header length (5 * 4 = 20 bytes)
    ip.iph_tos = 0;             // Type of service
    ip.iph_len = htons(sizeof(struct ip_header) + 20);  // Total length (IP header + data)
    ip.iph_ident = htons(54321);  // Packet ID
    ip.iph_ttl = 64;            // Time-to-live
    ip.iph_protocol = IPPROTO_ICMP; // Protocol ICMP (just as an example, could be any protocol)
    
    // Set spoofed source and destination IP addresses
    ip.iph_sourceip.s_addr = inet_addr("192.168.1.100"); // Spoofed source IP
    ip.iph_destip.s_addr = inet_addr("192.168.1.2");    // Destination IP

    ip.iph_chksum = checksum((unsigned short *)&ip, sizeof(struct ip_header)); // IP checksum

    // Prepare the packet to send (Ethernet header + IP header)
    memcpy(packet, &eth, sizeof(eth));
    memcpy(packet + sizeof(eth), &ip, sizeof(ip));

    // Setup sockaddr_in structure for destination (no port needed for raw socket)
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_addr.s_addr = ip.iph_destip.s_addr;

    // Send the packet
    if (sendto(sockfd, packet, sizeof(eth) + sizeof(ip), 0, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) < 0) {
        perror("Send failed");
        return 1;
    }

    printf("IP spoofed packet sent successfully!\n");

    close(sockfd);
    return 0;
}
