#include "client_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <root_dir> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *root_dir = argv[1];
    int port = atoi(argv[2]);

    print_server_ip();
    printf("Server root directory: %s\n", root_dir);
    printf("Listening on port %d...\n", port);

    signal(SIGCHLD, SIG_IGN);

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) == -1) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(server_sock);
            handle_client(client_sock, root_dir);
        } else if (pid > 0) {
            close(client_sock);
        }
    }

    close(server_sock);
    return 0;
}
