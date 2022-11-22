#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>

#ifndef UTILS_H
#define UTILS_H

int Accept(int, struct sockaddr*, socklen_t*);

int Bind(int, const struct sockaddr*, socklen_t);

int Connect(int, const struct sockaddr*, socklen_t);

int Close(int);

FILE* Fopen(char*, char*);

int Fputs(char*, FILE*);

void Getpeername(int, struct sockaddr*, socklen_t*);

int Getsockname(int, struct sockaddr*, socklen_t*);

int Inet_pton(int, const char*, void*);

void Inet_ntop(int, const void*, char*, socklen_t);

int Listen(int, int);

int Recv(int, char*, size_t, int);

int Select(int, fd_set*, fd_set*, fd_set*, struct timeval*);

int Shutdown(int, int);

int Socket(int, int, int);

int Write(int, const char*, size_t);

#endif