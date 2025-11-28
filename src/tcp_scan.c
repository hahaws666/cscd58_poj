#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "monitor.h"

int scan_port(const char *host, int port) {
    struct addrinfo hints;
    struct addrinfo *res;
    char portbuf[16];
    snprintf(portbuf, sizeof(portbuf), "%d", port);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // cannnot find address information
    // https://elixir.bootlin.com/linux/v6.11.6/source/drivers/tty/serial/8250/8250_port.c#L54
    if (getaddrinfo(host, portbuf, &hints, &res) != 0) return PORT_UNKNOWN;
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // connect to socket
    printf("1111111111111\n");
    int ret = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (ret == 0) {
        close(sockfd);
        freeaddrinfo(res);
        return PORT_OPEN;
    }

    // https://man7.org/linux/man-pages/man2/select.2.html
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);

    // https://man7.org/linux/man-pages/man3/timeval.3type.html
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_sec = 0;
    printf("2222222222222222\n");
    ret = select(sockfd + 1, NULL, &wfds, NULL, &tv);
    printf("33333333333333333333\n");
    int err = 0;
    //socklen_t len = sizeof(int);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &(sizeof(int)));
    close(sockfd);
    freeaddrinfo(res);
    if (err == 0) return PORT_OPEN;
    else if (err == ECONNREFUSED) return PORT_CLOSED;
    else return PORT_UNKNOWN;
}
