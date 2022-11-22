#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "utils.h"

int Accept(int socket, struct sockaddr *clientaddr, socklen_t *clientlen)
{
    int connfd;
    if ((connfd = accept(socket, (struct sockaddr *)NULL, NULL)) == -1)
    {
        perror("accept");
        exit(1);
    }
    return connfd;
}

int Bind(int socket, const struct sockaddr *servaddr, socklen_t servlen)
{
    int result;
    if ((result = bind(socket, servaddr, servlen)) == -1)
    {
        perror("bind");
        exit(1);
    }
    return result;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int result;
    if ((result = connect(sockfd, addr, addrlen)) < 0)
    {
        perror("connect error");
        exit(1);
    }
    return result;
}

int Close(int fd)
{
    int result;
    if ((result = close(fd)) == -1)
    {
        perror("close");
        exit(1);
    }
    return result;
}

FILE* Fopen(char *filename, char *mode) {
    FILE* fd;
    if ((fd = fopen(filename, mode)) == NULL)
    {
        perror("fopen");
        exit(1);
    }
    return fd;
}

int Fputs(char* buf, FILE* fd) {
    int result;
    if ((result = fputs(buf, fd)) == EOF)
    {
        perror("fputs");
        exit(1);
    }
    return result;

}

void Getpeername(int connfd, struct sockaddr *cliaddr, socklen_t *addrlen)
{
    if (getpeername(connfd, cliaddr, addrlen) == -1)
    {
        perror("getpeername");
        exit(1);
    }
}

int Getsockname(int socket, struct sockaddr *servaddr, socklen_t *addrlen)
{
    int result;
    if ((result = getsockname(socket, servaddr, addrlen)) == -1)
    {
        perror("getsockname");
        exit(1);
    }
    return result;
}

int Inet_pton(int af, const char *src, void *dst) {
    int result;
    if ((result = inet_pton(af, src, dst)) <= 0)
    {
        perror("inet_pton error");
        exit(1);
    }
    return result;
}

void Inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (inet_ntop(AF_INET, src, dst, size) <= 0)
    {
        perror("inet_ntop");
        exit(1);
    }
}

int Listen(int socket, int queueSize)
{
    if (listen(socket, queueSize) == -1)
    {
        perror("listen");
        exit(1);
    }
    return 0;
}

int Recv(int fd, char *buf, size_t buf_size, int flags) {
    int result;
    if ((result = recv(fd, buf, buf_size, flags)) == -1) {
        perror("recv");
        exit(1);
    }
    return result;
}

int Select(int maxfdp1, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    int result;
    if ((result = select(maxfdp1, readfds, writefds, exceptfds, timeout)) == -1) {
        perror("select");
        exit(1);
    }
    return result;
}

int Shutdown(int fd, int how) {
    int result;
    if ((result = shutdown(fd, how)) == -1) {
        perror("shutdown");
        exit(1);
    }
    return result;
}

int Socket(int family, int type, int flags)
{
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0)
    {
        perror("socket");
        exit(1);
    }
    else
    {
        return sockfd;
    }
}

int Write(int fd, const char* string, size_t size) {
    int result;
    if ((result = write(fd, string, size)) == -1) {
        perror("write");
        exit(1);
    }
    return result;
}