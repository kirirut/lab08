// client_handler.h
#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

void handle_client(int client_sock, const char *root_dir);
void cleanup_clients();

#endif