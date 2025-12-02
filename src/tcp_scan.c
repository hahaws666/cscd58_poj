#define _GNU_SOURCE
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
    // printf("1111111111111 start of scan port!!\n");
    struct addrinfo hints;
    struct addrinfo *res;
    char portbuf[16];
    snprintf(portbuf, sizeof(portbuf), "%d", port);
    // taking so much time debug to realize we must need intialize to 0 for every addrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // https://elixir.bootlin.com/linux/v6.11.6/source/drivers/tty/serial/8250/8250_port.c#L54
    // cannnot find address information
    //printf("1111111111111 passing before getaddrinfo...\n");
    if (getaddrinfo(host, portbuf, &hints, &res) != 0) return 0;
    // printf("1111111111111 passing over getaddrinfo...\n");
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) return 0;
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    // connect to socket
    int ret = connect(sockfd, res->ai_addr, res->ai_addrlen);
    // check if we can early return the answer!
    if (ret == 0) {
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }
    // sad case
    if (ret < 0 && errno != EINPROGRESS) {
        close(sockfd);
        freeaddrinfo(res);
        if (errno == ECONNREFUSED) return 2;
        else {
            return 0;
        }
    }
    // https://man7.org/linux/man-pages/man2/select.2.html
    fd_set ans_fd;
    // https://linux.die.net/man/3/fd_set
    FD_ZERO(&ans_fd);
    FD_SET(sockfd, &ans_fd);
    // https://man7.org/linux/man-pages/man3/timeval.3type.html
    struct timeval tv;
    // not sure if we need this?
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    //printf("2222222222222222 now we go to the select socket again\n");
    ret = select(sockfd + 1, NULL, &ans_fd, NULL, &tv);
    if (ret <= 0) {
        close(sockfd);
        freeaddrinfo(res);
        return 0;
    }
    //printf("33333333333333333333\n");
    int err = 0;
    socklen_t len = sizeof(int);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len)) {
        close(sockfd);
        freeaddrinfo(res);
        return 0;
    }
    close(sockfd);
    freeaddrinfo(res);
    // printf("33333333333333333333 At the end!!!\n");
    if (err == 0) return 1;
    else if (err == ECONNREFUSED) return 2;
    else return 0;
}
