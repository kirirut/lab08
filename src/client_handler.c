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

void handle_client(int client_sock, const char *root_dir) {
    char buffer[1024];
    ssize_t bytes_read;

    if (chdir(root_dir) != 0) {
        perror("chdir");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    char current_dir[PATH_MAX];
    char full_path[PATH_MAX];  

    while ((bytes_read = read(client_sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';  
    
        log_event(buffer);  
    
        if (strncmp(buffer, "ECHO ", 5) == 0) {
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
                     "PID: %d\nHost: %s\nOS: %s\nCurrent Dir: %s\n",
                     getpid(), uts.nodename, uts.sysname, current_dir);
            write(client_sock, info, strlen(info));
        } else if (strncmp(buffer, "CD ", 3) == 0) {
            char *path = buffer + 3;
            snprintf(full_path, sizeof(full_path), "%s/%s", root_dir, path);
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
                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        write(client_sock, entry->d_name, strlen(entry->d_name));
                        write(client_sock, "\n", 1);
                    }
                }
                closedir(dir);
            } else {
                write(client_sock, "FAIL\n", 5);
            }
        } else {
            write(client_sock, "UNKNOWN COMMAND\n", 16);
        }
    }

    close(client_sock);
    log_event("Client disconnected");
    exit(0);
}
