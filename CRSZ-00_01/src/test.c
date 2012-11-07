#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>

#include "main.h"
#include "server.h"
#include "protocol.h"

static int serialport;



void fail_with_msg(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void init_serial(int fd) {
	struct termios oldtio,newtio;

	tcgetattr(fd,&oldtio);
	bzero(&newtio, sizeof(newtio));


	newtio.c_cflag &= ~PARENB;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag = CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag &= ~(INPCK | ISTRIP);
	newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_oflag &= ~OPOST;

	cfsetispeed(&newtio, BAUDRATE);
	cfsetospeed(&newtio, BAUDRATE);

	newtio.c_cc[VTIME] = 4;
	newtio.c_cc[VMIN] = 0;

	if (tcsetattr(fd,TCSANOW,&newtio) < 0) {
		fail_with_msg("tcsetattr");
	}

	tcflush(fd, TCIFLUSH);
}


int on_socket_data(const char *socket_data, int length, int fd) {
	int n;

	if (length > 2) {
		switch (socket_data[0]) {
			case '0':
			if (cmd_dim(serialport, socket_data+2, length-2) < 0) { // Skip ;
				n = write(fd, "error", 5);
				if (n <= 0) {
					return -1;
				}
				return 0;
			}
			break;
			
			default:
			n = write(fd, "error", 5);
			if (n <= 0) {
				return -1;
			}
			return 0;
		}
	}
	else {
		n = write(fd, "error", 5);
		if (n <= 0) {
			return -1;
		}
		return 0;
	}

	// Socket response
	n = write(fd, "ok", 2);
	if (n <= 0) {
		return -1;
	}
	return 0;
}

int main(int argc, char ** argv) {
	struct sigaction sa;

	serialport = open(SERIALDEVICE, O_RDWR | O_NOCTTY);
	if (serialport < 0) {
		perror(SERIALDEVICE);
		exit(EXIT_FAILURE);
	}

	init_serial(serialport);
	send_init_commands(serialport);

	// Signal handler
	sa.sa_handler = shutdown_server;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		exit(EXIT_FAILURE);
	}

	init_server();
	fprintf(stderr, "Ready to serve\n");
	get_server_data_loop(&on_socket_data);

	fprintf(stderr, "Shutting down\n");
	send_shutdown_command(serialport);
	close(serialport);

	exit(EXIT_SUCCESS);
}
