#include "client_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int connect_to_server(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    printf("Connected to server at %s:%d\n", ip, port);
    return sock;
}

void run_client_loop(int sock) {
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            perror("recv");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }

    close(sock);
    printf("Connection closed.\n");
}
