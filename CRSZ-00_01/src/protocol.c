#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

static int sequence = 0;

void read_ignore_response(int fd) {
	int pkt_len;
	unsigned char serbuf[255];
	int i;
	int success = 1;

	size_t bytes_read = read(fd, serbuf, 1);
	
	if (bytes_read == 1 && serbuf[0] == 0x5a) {
		bytes_read = read(fd, serbuf, 1);
		if (bytes_read == 1) {
			pkt_len = serbuf[0];
			//fprintf(stderr, "Packet length: %d\n", pkt_len);

			for (i=0; i<pkt_len - 2; i++) {
				bytes_read = read(fd, serbuf, 1);
				if (bytes_read != 1) {
					fprintf(stderr, "Premature end of transmission while reading %d bytes\n", pkt_len);
					success = FALSE;
					break;
				}
				//fprintf(stderr, "%u, %02x\n", (unsigned int)bytes_read, (unsigned char)serbuf[0]);
			}

			if (success && serbuf[0] != 0xa5) {
				fprintf(stderr, "Unexpected end of transmission byte (was %02X)\n", serbuf[0]);
			}
		}
		else {
			fprintf(stderr, "Unexpected end of transmission\n");
		}
	}
	else {
		fprintf(stderr, "Empty read\n");
	}
}

// At least some of these commands seem to be needed first, in order to get a response from the set dim level command...
void send_init_commands(int fd) {
	char cmd1[] = "\x5A\x04\x0B\xA5";
	char cmd2[] = "\x5A\x09\x47\x08\x00\x00\x00\x06\xA5";
	char cmd3[] = "\x5A\x04\x44\xA5";
	char cmd4[] = "\x5A\x04\x42\xA5";
	char cmd5[] = "\x5A\x04\x22\xA5";
	char cmd6[] = "\x5A\x05\x33\x01\xA5";
	char cmd7[] = "\x5A\x05\x1A\x01\xA5";
	char cmd8[] = "\x5A\x06\x34\xFF\xFF\xA5";
	char cmd9[] = "\x5A\x04\x31\xA5";
	char cmd10[] = "\x5A\x04\x36\xA5";

	if (write(fd, cmd1, sizeof(cmd1)-1) != sizeof(cmd1)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd2, sizeof(cmd2)-1) != sizeof(cmd2)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd3, sizeof(cmd3)-1) != sizeof(cmd3)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd4, sizeof(cmd4)-1) != sizeof(cmd4)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd5, sizeof(cmd5)-1) != sizeof(cmd5)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd6, sizeof(cmd6)-1) != sizeof(cmd6)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd7, sizeof(cmd7)-1) != sizeof(cmd7)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd8, sizeof(cmd8)-1) != sizeof(cmd8)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd9, sizeof(cmd9)-1) != sizeof(cmd9)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);

	if (write(fd, cmd10, sizeof(cmd10)-1) != sizeof(cmd10)-1)
		fail_with_msg("Failed to write");
	
	read_ignore_response(fd);
}

void send_shutdown_command(int fd) {
	char cmd[] = "\x5A\x05\x1A\x00\xA5";
	
	if (write(fd, cmd, sizeof(cmd)-1) != sizeof(cmd)-1)
		fail_with_msg("Failed to write");
}


int cmd_dim(int serialport, const char *data, int length) {
	int i = 0;
	char *data2 = malloc(length+1);
	long serial_num = -1;
	int level = -1;

	if (!data2) {
		return -1;
	}

	memcpy(data2, data, length);
	data2[length] = 0;

	fprintf(stderr, "Parsing cmd...\n");
	char *token = data2;
	while ((token = strtok(token, ";"))) {
		fprintf(stderr, "Token: %s\n", token);
		switch (i) {
			case 0:
			serial_num = strtol(token, NULL, 10);
			break;

			case 1:
			level = strtol(token, NULL, 10);
			break;

			default:
			return -1;
		}
		i++;
		token = NULL;
	}

	if (level > 255 || level <= 0) {
		return -1;
	}
	if (serial_num <= 0) {
		return -1;
	}

	fprintf(stderr, "%ld %d\n", serial_num, level);

	unsigned char buf[] = "\x5a\x19\x1B\x5A\x00\x15\x00\x82\x07\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\xA5";

	size_t bufsize = sizeof(buf)-1;

	// OFF
	//char type2 = 0x51;

	// ON
	//char type2 = 0x50;

	buf[6] = 0x10 | ((unsigned int)(sequence%10) & 0x0f);
	buf[15] = serial_num & 0xff;
	buf[16] = (serial_num >> 8) & 0xff;
	buf[17] = (serial_num >> 16) & 0xff;
	buf[18] = (serial_num >> 24) & 0xff;
	buf[21] = level;
	buf[1] = bufsize;
	buf[3] = 0x5a; // DIM LEVEL

	// Start at 3 and skip last 3
	unsigned short crc = calculate_crc(buf+3, bufsize-3-3);

	buf[22] = crc & 0xff;
	buf[23] = (crc >> 8) & 0xff;

	write(serialport, buf, sizeof(buf)-1);
	read_ignore_response(serialport);

	sequence++;
	
	usleep(30000);
	
	return 0;
}
