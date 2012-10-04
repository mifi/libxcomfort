#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "lib_crc/lib_crc.h"

#define SERIALDEVICE "/dev/ttyUSB0"
#define BAUDRATE B115200

#define TRUE 1
#define FALSE 0

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
			fprintf(stderr, "Packet length: %d\n", pkt_len);

			for (i=0; i<pkt_len - 2; i++) {
				bytes_read = read(fd, serbuf, 1);
				if (bytes_read != 1) {
					fprintf(stderr, "unexpected eot2\n");
					success = FALSE;
					break;
				}
				//fprintf(stderr, "%u, %02x\n", (unsigned int)bytes_read, (unsigned char)serbuf[0]);
			}

			if (success && serbuf[0] != 0xa5) {
				fprintf(stderr, "unexpected eot byte (was %02X)\n", serbuf[0]);
			}
		}
		else {
			fprintf(stderr, "unexpected eot1\n");
		}
	}
	else {
		fprintf(stderr, "read nothing\n");
	}
}


void fail_with_msg(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

// At least some of these commands seem to be needed in order to get a response from the set dim level command...
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

unsigned short calculate_crc(char *buf, size_t len) {
	unsigned short crc = 0;

	size_t i=0;
	for (i=0; i<len; i++) {
		crc = update_crc_kermit(crc, buf[i]);
	}
		
	return crc;
}

void init_serial(int fd) {
	struct termios oldtio,newtio;
	int status;

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

	/*if (ioctl(fd, TIOCMGET, &status) == -1) {
		fail_with_msg("ioctl");
	}
	//status |= TIOCM_DTR;
	//status |= TIOCM_RTS;
	//status |= TIOCM_CTS;
	if (ioctl(fd, TIOCMSET, &status) == -1) {
		fail_with_msg("ioctl");
	}*/
}

void main(int argc, char ** argv) {
	char serbuf[255];
	int pkt_len;
	size_t bytes_read;
	unsigned char buf[] = "\x5a\x19\x1B\x5A\x00\x15\x00\x82\x07\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\xA5";
	size_t bufsize = sizeof(buf)-1;

	int i;
	unsigned long serial_num_living_room = 4161029;
	unsigned long serial_num_kitchen = 4160956;
	unsigned long serial_num_bedroom = 4161034;
	unsigned int serial_num = serial_num_living_room;
	int level = -1;
	int seq = -1;

	int option = 0;
	while ((option = getopt(argc, argv,"l:s:")) != -1) {
		switch (option) {
			case 'l':
				level = atoi(optarg);
			case 's':
				seq = atoi(optarg);
			break;
			default:
				exit(EXIT_FAILURE);
		}
	}

	if (level < 0 || level > 255) {
		fail_with_msg("Invalid level");
	}
	if (seq < 0 || seq > 0x0f) {
		fail_with_msg("Invalid seq");
	}

	// DIM LEVEL
	char type1 = 0x19;
	char type2 = 0x5a;

	// OFF
	//char type1 = 0x17;
	//char type2 = 0x51;
	
	// ON
	//char type1 = 0x17;
	//char type2 = 0x50;

	buf[1] = type1;
	buf[6] = 0x10 | ((unsigned int)seq & 0x0f);
	buf[15] = serial_num & 0xff;
	buf[16] = (serial_num >> 8) & 0xff;
	buf[17] = (serial_num >> 16) & 0xff;
	buf[18] = (serial_num >> 24) & 0xff;
	buf[21] = level;

	buf[1] = bufsize;

	// Start at 3 and skip last 3
	unsigned short crc = calculate_crc(buf+3, bufsize-3-3);

	buf[22] = crc & 0xff;
	buf[23] = (crc >> 8) & 0xff;

	/*for (i=0; i<bufsize; i++) {
		printf("%.2X", buf[i]);
	}
	printf("\n");*/

	int serialport = open(SERIALDEVICE, O_RDWR | O_NOCTTY);
	if (serialport < 0) {
		perror(SERIALDEVICE);
		exit(EXIT_FAILURE);
	}

	init_serial(serialport);
	send_init_commands(serialport);
	
	usleep(100000);
	
	write(serialport, buf, sizeof(buf)-1);
	read_ignore_response(serialport);

	usleep(100000);

	send_shutdown_command(serialport);
	close(serialport);
}
