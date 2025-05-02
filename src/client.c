#include <stdio.h>
#include <stdlib.h>
#include "client_func.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = connect_to_server(server_ip, port);
    if (sock < 0) {
        return EXIT_FAILURE;
    }

    run_client_loop(sock);
    return EXIT_SUCCESS;
}
