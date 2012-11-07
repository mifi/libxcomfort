#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

static int sockfd;
static int running = 1;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void get_server_data_loop(int (*on_data)(const char *, int, int)) {
	size_t n;
	int newsockfd = -1;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	char buffer[255];
	int buf_len = sizeof(buffer);
	int offset = 0;
	int reaccept = 1;
	const char *delim;
	int actual_len;

	while (running) {
		if (reaccept) {
			if (newsockfd > 0) {
				close(newsockfd);
			}

			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) {
				reaccept = 1;
				continue;
			}
			reaccept = 0;
		}

		offset = 0;
		do {
			n = read(newsockfd, buffer + offset, buf_len - offset);
			if (n <= 0) {
				reaccept = 1;
				break;
			}

			offset += n;
			delim = memchr(buffer, '.', offset);
		}
		while (delim == NULL && buf_len - offset > 0);

		if (!reaccept) {
			if (delim == NULL) {
				fprintf(stderr, "Too long packet received\n");
				reaccept = 1;
			}

			actual_len = delim - buffer;
			if (offset > actual_len + 1) {
				fprintf(stderr, "(TODO) Read %d extra bytes after delim\n", offset - (actual_len + 1));
			}

			if (on_data(buffer, actual_len, newsockfd) < 0) {
				reaccept = 1;
				continue;
			}
		}
	}

	if (newsockfd > 0) {
		close(newsockfd);
	}
	close(sockfd);
}

int init_server()
{
	int portno;
	struct sockaddr_in serv_addr;
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	memset(&serv_addr, 0, sizeof(serv_addr));
	portno = LISTEN_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

	if (listen(sockfd, BACKLOG) < 0) {
		error("listen");
	}

	return 0; 
}

void shutdown_server() {
	running = 0;
}
