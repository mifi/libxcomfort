#ifndef SERVER_H
#define SERVER_H

#define LISTEN_PORT 1050
#define BACKLOG 5

void get_server_data_loop();
int init_server();
void shutdown_server();

#endif
