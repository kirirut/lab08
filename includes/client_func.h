#ifndef CLIENT_FUNC_H
#define CLIENT_FUNC_H
int connect_to_server(const char *ip, int port);
void run_client_loop(int sock);

#endif 
