#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define max(a, b) (((a)>(b))?(a):(b))

#define SERVER_PORT 8081
static const char* server_address = "127.0.0.1";
static const char* END_MSG = "finalizar_chat";
static const int END_MSG_LEN = 14;

void getTimeAsString(char* str, int strlen, char* fmt)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(str, strlen, fmt, tm);
}

int main(int argc, char const *argv[]) {
    int udp_fd;
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = 0;

    if ((bind(udp_fd, (struct sockaddr*)&udp_addr, sizeof(udp_addr))) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in serv_addr;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_address, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getsockname(udp_fd, (struct sockaddr*)&udp_addr, &addrlen) < 0) {
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

    int n = send(sock_fd, &udp_addr.sin_port, sizeof(in_port_t), 0);
    if (n < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    int user_fd = fileno(stdin);

    struct sockaddr_in peer_addr;

    while (true) {
        while (true) {
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(user_fd, &fdset);
            FD_SET(udp_fd, &fdset);
            FD_SET(sock_fd, &fdset);
            
            int nfds = max(max(user_fd, udp_fd), sock_fd) + 1;

            int n = select(nfds, &fdset, NULL, NULL, NULL);
            if (n == -1) {
                perror("select failed");
                exit(EXIT_FAILURE);
            }

            if (FD_ISSET(udp_fd, &fdset)) {
                char ping;

                int n = recvfrom(udp_fd, &ping, 1, 0, (struct sockaddr*)&peer_addr, &addrlen);
                if (n < 0) {
                    perror("recv failed");
                    exit(EXIT_FAILURE);
                }

                break;
            }
            
            if (FD_ISSET(user_fd, &fdset)) {
                int target;
                scanf("%d", &target);

                int bytes_sent = send(sock_fd, &target, sizeof(int), 0);
                if (bytes_sent < 0) {
                    perror("send failed");
                    exit(EXIT_FAILURE);
                }

                recv(sock_fd, &peer_addr, sizeof(peer_addr), 0);
                break;
            }

            if (FD_ISSET(sock_fd, &fdset)) {
                char client_list_buf[1024] = {0};
                int bytes_received = recv(sock_fd, client_list_buf, 1024, 0);
                if (bytes_received < 0) {
                    perror("recv failed");
                    exit(EXIT_FAILURE);
                }
                printf("Clientes conectados:\n%s\n", client_list_buf);

                printf("Escolha alguém para se conectar: ");
                fflush(stdout);
            }
        }

        char* target_ip = inet_ntoa(peer_addr.sin_addr);
        if (target_ip == NULL) {
            printf("Endereço IP inválido\n");
            exit(EXIT_FAILURE);
        }

        int target_port = ntohs(peer_addr.sin_port);
        printf("Conectado a %s:%d", target_ip, target_port);

        char message[1024] = {0};

        printf("> ");
        fflush(stdout);

        char timestr[64];
        FILE* outputfp;
        getTimeAsString(timestr, 64, "%d%m_%H%M%S");
        char str[2048];
        sprintf(str, "chat_%d_%s", getpid(),timestr);
        outputfp = fopen(str, "w");
        getTimeAsString(timestr, 64, "%F %T");
        sprintf(str, "Chat iniciado as %s", timestr);
        fputs(str, outputfp);

        while (true) {
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(user_fd, &fdset);
            FD_SET(udp_fd, &fdset);
            int nfds = max(user_fd, udp_fd) + 1;

            int n = select(nfds, &fdset, NULL, NULL, NULL);
            if (n == -1) {
                perror("select failed");
                exit(EXIT_FAILURE);
            }

            if (FD_ISSET(udp_fd, &fdset)) {
                int n = recvfrom(udp_fd, message, 1024, 0, (struct sockaddr*)&peer_addr, &addrlen);
                if (n < 0) {
                    perror("recv failed");
                    exit(EXIT_FAILURE);
                }

                char* sender_ip = inet_ntoa(peer_addr.sin_addr);
                int sender_port = ntohs(peer_addr.sin_port);

                if (sender_ip == NULL) {
                    printf("invalid chat message received\n");
                    exit(EXIT_FAILURE);
                }

                sprintf(str, "\rMensagem recebida de %s:%d: %s\n> ", sender_ip, sender_port, message);
                printf(str);
                fputs(str, outputfp);

                if (strncmp(message, END_MSG, END_MSG_LEN) == 0) {
                    send(sock_fd, END_MSG, END_MSG_LEN, 0);
                    break;
                }
            }

            if (FD_ISSET(0, &fdset)) {
                printf("> ");
                fflush(stdout);

                if (fgets(message, 1024, stdin) == NULL) {
                    continue;
                }

                int n = sendto(udp_fd, message, strlen(message), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
                if (n < 0) {
                    perror("send failed");
                    exit(EXIT_FAILURE);
                }

                if (strlen(message)) {
                    sprintf(str, "\r> %s\n", message);
                    fputs(str, outputfp);
                }
                
                if (strncmp(message, END_MSG, END_MSG_LEN) == 0) {
                    send(sock_fd, END_MSG, END_MSG_LEN, 0);
                    break;
                }
            }
        }
        fflush(outputfp);
        getTimeAsString(timestr, 64, "%F %T");
        sprintf(str, "Chat finalizado as %s", timestr);
        fputs(str, outputfp);
        fclose(outputfp);
    }

    return 0;
}
