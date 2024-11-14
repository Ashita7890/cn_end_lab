#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void *process_tcp_packets(void *arg);
//void *process_udp_packets(void *arg);
void *process_icmp_packets(void *arg);
void process_packet(unsigned char *buffer, int size);
void print_ip_header(unsigned char *buffer, int size);
void print_tcp_packet(unsigned char *buffer, int size);
//void print_udp_packet(unsigned char *buffer, int size);
void print_icmp_packet(unsigned char *buffer, int size);

// Buffer for receiving packets
unsigned char buffer[65536];

int main() {
    pthread_t tcp_thread, udp_thread, icmp_thread;

    // Create threads for each protocol
    pthread_create(&tcp_thread, NULL, process_tcp_packets, NULL);
    //pthread_create(&udp_thread, NULL, process_udp_packets, NULL);
    pthread_create(&icmp_thread, NULL, process_icmp_packets, NULL);

    // Wait for the threads to finish (this never happens in this example)
    pthread_join(tcp_thread, NULL);
    //pthread_join(udp_thread, NULL);
    pthread_join(icmp_thread, NULL);

    return 0;
}

// Function to process TCP packets in a separate thread
void *process_tcp_packets(void *arg) {
    int sock_raw;
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);

    // Create a raw socket for TCP
    sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock_raw < 0) {
        perror("TCP Socket Error");
        return NULL;
    }

    while (1) {
        int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, &saddr, (socklen_t *)&saddr_len);
        if (data_size < 0) {
            perror("Recvfrom TCP error");
            break;
        }
        process_packet(buffer, data_size);
    }

    close(sock_raw);
    return NULL;
}

// Function to process UDP packets in a separate thread
// void *process_udp_packets(void *arg) {
//     int sock_raw;
//     struct sockaddr saddr;
//     int saddr_len = sizeof(saddr);

//     // Create a raw socket for UDP
//     sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
//     if (sock_raw < 0) {
//         perror("UDP Socket Error");
//         return NULL;
//     }

//     while (1) {
//         int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, &saddr, (socklen_t *)&saddr_len);
//         if (data_size < 0) {
//             perror("Recvfrom UDP error");
//             break;
//         }
//         process_packet(buffer, data_size);
//     }

//     close(sock_raw);
//     return NULL;
// }

// Function to process ICMP packets in a separate thread
void *process_icmp_packets(void *arg) {
    int sock_raw;
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);

    // Create a raw socket for ICMP
    sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_raw < 0) {
        perror("ICMP Socket Error");
        return NULL;
    }

    while (1) {
        int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, &saddr, (socklen_t *)&saddr_len);
        if (data_size < 0) {
            perror("Recvfrom ICMP error");
            break;
        }
        process_packet(buffer, data_size);
    }

    close(sock_raw);
    return NULL;
}

// Process a captured packet based on its protocol
void process_packet(unsigned char *buffer, int size) {
    struct ip *iph = (struct ip *)buffer;

    switch (iph->ip_p) {
        case IPPROTO_TCP:
            printf("\nTCP Packet:\n");
            print_tcp_packet(buffer, size);
            break;

        // case IPPROTO_UDP:
        //     printf("\nUDP Packet:\n");
        //     print_udp_packet(buffer, size);
        //     break;

        case IPPROTO_ICMP:
            printf("\nICMP Packet:\n");
            print_icmp_packet(buffer, size);
            break;

        default:
            printf("\nOther Protocol: %d\n", iph->ip_p);
            print_ip_header(buffer, size);
            break;
    }
}

// Print IP Header
void print_ip_header(unsigned char *buffer, int size) {
    struct ip *iph = (struct ip *)buffer;
    struct sockaddr_in source, dest;

    source.sin_addr = iph->ip_src;
    dest.sin_addr = iph->ip_dst;

    printf("IP Header:\n");
    printf("   |-Source IP         : %s\n", inet_ntoa(source.sin_addr));
    printf("   |-Destination IP    : %s\n", inet_ntoa(dest.sin_addr));
    printf("   |-TTL               : %d\n", (unsigned int)iph->ip_ttl);
    printf("   |-Protocol          : %d\n", (unsigned int)iph->ip_p);
}

// Print TCP Packet
void print_tcp_packet(unsigned char *buffer, int size) {
    struct ip *iph = (struct ip *)buffer;
    unsigned short iphdrlen = iph->ip_hl * 4;

    struct tcphdr *tcph = (struct tcphdr *)(buffer + iphdrlen);

    printf("TCP Header:\n");
    printf("   |-Source Port      : %u\n", ntohs(tcph->th_sport));  // Correct usage
    printf("   |-Destination Port : %u\n", ntohs(tcph->th_dport));    // Correct usage
}

// Print UDP Packet
// void print_udp_packet(unsigned char *buffer, int size) {
//     struct ip *iph = (struct ip *)buffer;
//     unsigned short iphdrlen = iph->ip_hl * 4;

//     struct udphdr *udph = (struct udphdr *)(buffer + iphdrlen);

//     printf("UDP Header:\n");
//     printf("   |-Source Port      : %d\n", ntohs(udph->th_source));
//     printf("   |-Destination Port : %d\n", ntohs(udph->dest));
// }

// Print ICMP Packet
void print_icmp_packet(unsigned char *buffer, int size) {
    struct ip *iph = (struct ip *)buffer;
    unsigned short iphdrlen = iph->ip_hl * 4;

    struct icmp *icmph = (struct icmp *)(buffer + iphdrlen);

    printf("ICMP Header:\n");
    printf("   |-Type       : %d\n", icmph->icmp_type);
}
