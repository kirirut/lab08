#include "client_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 4096

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
    char input[BUFFER_SIZE];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        if (strcmp(input, "quit") == 0) {
            break;
        }

        size_t input_len = strlen(input);
        if (input_len >= BUFFER_SIZE - 1) {
            printf("Command too long\n");
            continue;
        }
        input[input_len] = '\n';
        input[input_len + 1] = '\0';

        if (send(sock, input, input_len + 1, 0) < 0) {
            perror("send");
            break;
        }

        memset(buffer, 0, sizeof(buffer));
        ssize_t total_bytes = 0;

        while (1) {
            ssize_t bytes_received = recv(sock, buffer + total_bytes, sizeof(buffer) - total_bytes - 1, 0);
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Server closed connection.\n");
                } else {
                    perror("recv");
                }
                close(sock);
                return;
            }

            total_bytes += bytes_received;
            buffer[total_bytes] = '\0';

            // Check if we've received a complete response
            if (total_bytes > 0 && (buffer[total_bytes - 1] == '\n' || total_bytes >= sizeof(buffer) - 1)) {
                break;
            }
        }

        // Print server response
        printf("%s", buffer);
        fflush(stdout);
    }

    close(sock);
    printf("Connection closed.\n");
}