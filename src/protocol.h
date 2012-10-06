void read_ignore_response(int fd);
void send_init_commands(int fd);
void send_shutdown_command(int fd);
int cmd_dim(int serialport, const char *data, int length);
