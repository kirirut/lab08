#include "client_handler.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <sys/stat.h>

static volatile int active_clients = 0;

void handle_client(int client_sock, const char *root_dir) {
    char buffer[4096];
    ssize_t bytes_read;

    if (chdir(root_dir) != 0) {
        perror("chdir");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    __sync_fetch_and_add(&active_clients, 1);

    char current_dir[PATH_MAX];
    char full_path[PATH_MAX];

    while ((bytes_read = read(client_sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (strlen(buffer) == 0) {
            write(client_sock, "EMPTY COMMAND\n", 14);
            log_event("Received empty command");
            continue;
        }

        log_event(buffer);

        if (strcmp(buffer, "HELP") == 0) {
            const char *help_msg =
                "Available commands:\n"
                "ECHO <message> - Echo back the message\n"
                "QUIT - Disconnect from server\n"
                "INFO - Show server information\n"
                "CD <path> - Change directory\n"
                "LIST - List directory contents\n"
                "HELP - Show this help message\n";
            write(client_sock, help_msg, strlen(help_msg));
        } else if (strncmp(buffer, "ECHO ", 5) == 0) {
            const char *msg = buffer + 5;
            write(client_sock, msg, strlen(msg));
            write(client_sock, "\n", 1);
        } else if (strcmp(buffer, "QUIT") == 0) {
            break;
        } else if (strcmp(buffer, "INFO") == 0) {
            struct utsname uts;
            uname(&uts);
            getcwd(current_dir, sizeof(current_dir));
            char info[512];
            snprintf(info, sizeof(info),
                     "Welcome to the File Server!\n"
                     "PID: %d\nHost: %.64s\nOS: %.64s\nCurrent Dir: %.256s\n",
                     getpid(), uts.nodename, uts.sysname, current_dir);
            write(client_sock, info, strlen(info));
        } else if (strncmp(buffer, "CD ", 3) == 0) {
            char *path = buffer + 3;
            if (path[0] == '\0') {
                write(client_sock, "INVALID PATH\n", 13);
                continue;
            }
            if (!getcwd(current_dir, sizeof(current_dir))) {
                write(client_sock, "FAIL\n", 5);
                continue;
            }
            size_t current_dir_len = strlen(current_dir);
            size_t path_len = strlen(path);
            if (current_dir_len + 1 + path_len >= sizeof(full_path)) {
                write(client_sock, "PATH TOO LONG\n", 14);
                continue;
            }

            full_path[0] = '\0';
            strncat(full_path, current_dir, current_dir_len);
            strncat(full_path, "/", 1);
            strncat(full_path, path, path_len);
            if (realpath(full_path, current_dir) && is_inside_root(root_dir, current_dir)) {
                if (chdir(current_dir) == 0) {
                    write(client_sock, "OK\n", 3);
                } else {
                    write(client_sock, "FAIL\n", 5);
                }
            } else {
                write(client_sock, "ACCESS DENIED\n", 14);
            }
        } else if (strcmp(buffer, "LIST") == 0) {
            DIR *dir = opendir(".");
            if (dir) {
                struct dirent *entry;
                char list_buffer[4096] = {0};
                size_t offset = 0;
                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        size_t len = strlen(entry->d_name);
                        if (offset + len + 1 < sizeof(list_buffer)) {
                            memcpy(list_buffer + offset, entry->d_name, len);
                            list_buffer[offset + len] = '\n';
                            offset += len + 1;
                        }
                    }
                }
                closedir(dir);
                if (offset > 0) {
                    write(client_sock, list_buffer, offset);
                } else {
                    write(client_sock, "EMPTY\n", 6);
                }
            } else {
                write(client_sock, "FAIL\n", 5);
            }
        } else {
            write(client_sock, "UNKNOWN COMMAND\n", 16);
        }
    }

    close(client_sock);
    log_event("Client disconnected");
    __sync_fetch_and_sub(&active_clients, 1);
    exit(0);
}

void cleanup_clients() {
    while (active_clients > 0) {
        printf("Waiting for %d clients to disconnect...\n", active_clients);
        sleep(1);
    }
}