#define _GNU_SOURCE
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

void log_event(const char *event) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info = localtime(&ts.tv_sec);
    char time_buf[64];

    snprintf(time_buf, sizeof(time_buf), "%04d.%02d.%02d-%02d:%02d:%02d.%03ld",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             ts.tv_nsec / 1000000);

    printf("%s %s\n", time_buf, event);
    fflush(stdout);
}

int is_inside_root(const char *root, const char *path) {
    char abs_root[PATH_MAX];
    char abs_path[PATH_MAX];
    realpath(root, abs_root);
    realpath(path, abs_path);
    return strncmp(abs_root, abs_path, strlen(abs_root)) == 0;
}

void print_server_ip() {
    char hostname[256];
    struct hostent *host;
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }
    host = gethostbyname(hostname);
    if (host == NULL) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }
    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    if (addr_list[0] != NULL) {
        printf("Server IP address: %s\n", inet_ntoa(*addr_list[0]));
    }
}
