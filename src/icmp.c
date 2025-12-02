// https://stackoverflow.com/questions/5582211/what-does-define-gnu-source-imply a nice article for GNU use
// since POSIX feature are not enabled by default
#define _GNU_SOURCE
#include "monitor.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <unistd.h>

// helper function to do the checksum returns checksum
// this is equivalent to our a3 after some refactoring
static uint16_t cksum(void *buf, int len) {
    uint32_t ans = 0;
    uint16_t *length = buf;
    while (len > 1) {
        ans += *length;
        length++;
        len -= 2;
    }
    if (len == 1) ans += *(uint8_t *)length;
    ans = (ans >> 16) + (ans & 0xffff);
    ans += (ans >> 16);
    return ~ans;
}
// function that to the icmp ping stuffs
int icmp_ping(const char *host, double *rtt_ms) {
    char sendbuf[64];
    // https://sites.uclouvain.be/SystInfo/usr/include/netinet/ip_icmp.h.html
    memset(sendbuf, 0, sizeof(sendbuf));
    struct icmphdr *ans_icmp = (struct icmphdr *)sendbuf;
    // fill in information for icmp
    ans_icmp->type = ICMP_ECHO;
    ans_icmp->code = 0;
    ans_icmp->un.echo.id = htons(getpid() & 0xFFFF);
    ans_icmp->un.echo.sequence = htons(1);
    ans_icmp->checksum = cksum(ans_icmp, sizeof(struct icmphdr));
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket Creation Failed");
        return -1;
    }
    // https://en.cppreference.com/w/c/chrono/timespec
    // https://man7.org/linux/man-pages/man3/clock_gettime.3.html
    struct timespec ans1, ans2;
    clock_gettime(CLOCK_MONOTONIC, &ans1);
    // https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    struct addrinfo hints;
    struct addrinfo *res;
    // we must initialize to 0 for addrinfo!!!
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    // will make the host into an IP address
    int err = getaddrinfo(host, NULL, &hints, &res);
    if (err != 0) {
        fprintf(stderr, "DNS Error: %s\n", gai_strerror(err));
        return -1;
    }
    // https://pubs.opengroup.org/onlinepubs/009604499/functions/sendto.html
    // now we will send the icmp packet over raw socket
    // sendto(sockfd, sendbuf, sizeof(struct icmphdr), 0, res->ai_addr, res->ai_addrlen);
    if (sendto(sockfd, sendbuf, sizeof(struct icmphdr), 0, res->ai_addr, res->ai_addrlen) < 0) {
      perror("Sendto Failed"); 
      close(sockfd);
      freeaddrinfo(res);
      return -1;
    }
    char recvbuf[1024];
    struct sockaddr_in reply_addr;
    socklen_t reply_len = sizeof(reply_addr);
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    // https://pubs.opengroup.org/onlinepubs/009695099/functions/setsockopt.html
    // set a receive timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ssize_t n = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&reply_addr, &reply_len);
    // so sad that it is unreachable
    if (n < 0) {
        //printf("333333Now we are at timeout..\n");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }
    struct iphdr *ip_header = (struct iphdr *)recvbuf;
    int ip_header_len = ip_header->ihl * 4;
    struct icmphdr *recv_icmp = (struct icmphdr *)(recvbuf + ip_header_len);
    if (recv_icmp->type != ICMP_ECHOREPLY) {
        printf("ICMP Packet Error: Received Type %d (Code %d) from host %s\n", recv_icmp->type, recv_icmp->code, host);
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }
    clock_gettime(CLOCK_MONOTONIC, &ans2);
    // RTT measurement formulas
    *rtt_ms = (ans2.tv_sec - ans1.tv_sec) * 1000.0 + (ans2.tv_nsec - ans1.tv_nsec) / 1e6;
    close(sockfd);
    freeaddrinfo(res);
    return 0;
}