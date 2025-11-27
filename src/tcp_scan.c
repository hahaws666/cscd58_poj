// tcp_scan.c
#include "monitor.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

int scan_port(const char *host, int port) {
    struct addrinfo hints = {0}, *res;
    char portbuf[16];
    snprintf(portbuf, sizeof(portbuf), "%d", port);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, portbuf, &hints, &res) != 0)
        return PORT_UNKNOWN;

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return PORT_UNKNOWN;
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    int ret = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (ret == 0) {
        close(sockfd);
        freeaddrinfo(res);
        return PORT_OPEN;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);

    struct timeval tv = {1, 0};

    ret = select(sockfd + 1, NULL, &wfds, NULL, &tv);

    if (ret <= 0) {
        close(sockfd);
        freeaddrinfo(res);
        return PORT_TIMEOUT;
    }

    int err = 0;
    socklen_t len = sizeof(err);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len);

    close(sockfd);
    freeaddrinfo(res);

    if (err == 0)
        return PORT_OPEN;
    if (err == ECONNREFUSED)
        return PORT_CLOSED;
    return PORT_UNKNOWN;
}
