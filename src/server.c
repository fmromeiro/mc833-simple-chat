#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/mman.h>

#define PORT 8080
#define BACKLOG 10

static const char* END_CHAT_MSG = "finalizar_chat";
static const int END_MSG_LEN = 14;

struct client_info {
    int fd;
    struct sockaddr_in addr;
};

void handle_child_termination(int signal) {
    pid_t pid;
    int stat;
    if ((pid = wait(&stat)) < 0) {
        perror("wait");
    }
}

int main() {
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // set socket options
    // int opt = 1;
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int udp_fd;
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(udp_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, handle_child_termination);

    struct client_info* clients = mmap(NULL, 100 * (sizeof (struct client_info)), PROT_READ | PROT_WRITE, 
                                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *num_clients;
    num_clients = mmap(NULL, sizeof *num_clients, PROT_READ | PROT_WRITE, 
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    while (true) {
        int connfd;
        struct sockaddr_in client_address;
        int addrlen = sizeof(address);
        if ((connfd = accept(listenfd, (struct sockaddr*)&client_address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        in_port_t port;
        int n = recv(connfd, &port, sizeof(port), 0);
        if (n < 0) {
            perror("recv failed");
            exit(EXIT_FAILURE);
        }

        client_address.sin_port = port;

        clients[*num_clients].fd = connfd;
        clients[*num_clients].addr = client_address;
        *num_clients = *num_clients + 1;

        printf("Client connected: %d with port %d (of %d)\n", connfd, port, *num_clients);

        char client_list_buf[1024] = {0};
        int client_list_buf_index = 0;
        for (int i = 0; i < *num_clients; i++) {
            client_list_buf_index += sprintf(client_list_buf + client_list_buf_index, "%d\n", clients[i].fd);
        }

        for (int i = 0; i < *num_clients; i++) {
            send(clients[i].fd, client_list_buf, strlen(client_list_buf), 0);
        }

        int pid = fork();
        if (pid == 0) {
            close(listenfd);
            
            while (true) {
                int target;
                char message[1024];
                
                int n = recv(connfd, message, 1024, 0);
                if (n < 0) {
                    perror("recv failed");
                    exit(EXIT_FAILURE);
                }

                if (strncmp(message, END_CHAT_MSG, END_MSG_LEN) == 0) {
                    char client_list_buf[1024] = {0};
                    int client_list_buf_index = 0;
                    for (int i = 0; i < *num_clients; i++) {
                        client_list_buf_index += sprintf(client_list_buf + client_list_buf_index, "%d\n", clients[i].fd);
                    }

                    for (int i = 0; i < *num_clients; i++) {
                        send(clients[i].fd, client_list_buf, strlen(client_list_buf), 0);
                    }
                    continue;
                }

                target = (int)*message;

                printf("Connection request: %d (from client %d)\n", target, connfd);
                for (int i = 0; i < *num_clients; i++) {
                    if (clients[i].fd == target) {
                        int n = send(connfd, &clients[i].addr, sizeof(struct sockaddr_in), 0);
                        if (n < 0) {
                            perror("send failed");
                            exit(EXIT_FAILURE);
                        }
                        break;
                    }
                }
            }
        } else if (pid == -1) {
            perror("fork");
            continue;
        }
    }

    return 0;
}
