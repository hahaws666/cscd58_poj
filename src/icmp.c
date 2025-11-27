// icmp.c
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
#include <unistd.h>

static uint16_t checksum(void *buf, int len) {
    uint32_t sum = 0;
    uint16_t *ptr = buf;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(uint8_t *)ptr;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int icmp_ping(const char *host, double *rtt_ms) {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;

    if (getaddrinfo(host, NULL, &hints, &res) != 0)
        return -1;

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
        return -1;

    char sendbuf[64];
    struct icmphdr *icmp = (struct icmphdr *)sendbuf;
    memset(sendbuf, 0, sizeof(sendbuf));

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    icmp->un.echo.sequence = htons(1);
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr));

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    ssize_t sent = sendto(
        sockfd, sendbuf, sizeof(struct icmphdr), 0,
        res->ai_addr, res->ai_addrlen);

    if (sent < 0) {
        close(sockfd);
        return -1;
    }

    char recvbuf[1024];
    struct sockaddr_in reply_addr;
    socklen_t reply_len = sizeof(reply_addr);

    struct timeval tv = {1, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    ssize_t n = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0,
                         (struct sockaddr *)&reply_addr, &reply_len);

    if (n <= 0) {
        close(sockfd);
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    *rtt_ms =
        (t2.tv_sec - t1.tv_sec) * 1000.0 +
        (t2.tv_nsec - t1.tv_nsec) / 1e6;

    close(sockfd);
    freeaddrinfo(res);

    return 0;
}
