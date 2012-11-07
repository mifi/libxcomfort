#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#include "libxcomfort.h"


static void hexdump(const char *data, size_t len) {
	size_t i;

	for (i = 0; i < len; i++) {
		if (i > 0)
			fprintf(stderr, " ");
		fprintf(stderr, "%02x", data[i] & 0xff);
	}
	fprintf(stderr, "\n");
}

static void init_packet(union lxc_packet *pkt) {
	bzero(pkt->data, sizeof(pkt->data));
	pkt->cmd.size = 6;
	pkt->cmd.type = LXC_PKT_TYPE_OUT;
}

void lxc_init() {
	libusb_init(NULL);
}

void lxc_exit() {
	libusb_exit(NULL);
}

void lxc_close(libusb_device_handle *dev) {
	libusb_release_interface(dev, LXC_USB_INTERFACE);
	libusb_close(dev);
}

libusb_device_handle *lxc_open() {
	struct libusb_device_handle *dev = libusb_open_device_with_vid_pid(NULL, LXC_USB_VENDOR, LXC_USB_PRODUCT);
	int ret;

	if (dev == NULL) {
		fprintf(stderr, "Failed to find/open USB device\n");
		return dev;
	}

	ret = libusb_kernel_driver_active(dev, LXC_USB_INTERFACE);

	if (ret == 1) {
		ret = libusb_detach_kernel_driver(dev, LXC_USB_INTERFACE);
		if (ret != 0) {
			printf("Can't detact device's kernel driver %d\n", ret);
			libusb_close(dev);
			return NULL;
		}
	}

	ret = libusb_set_configuration(dev, 1);
	if (ret != 0) {
		printf("Can't set configuration on USB device: %d\n", ret);
		libusb_close(dev);
		return NULL;
	}

	ret = libusb_claim_interface(dev, LXC_USB_INTERFACE);
	if (ret != 0) {
		printf("Could not claim interface: %d\n", ret);
		libusb_close(dev);
		return NULL;
	}

	return dev;
}

/**
 * Just for testing
 **/
void lxc_send_cmd_test(struct libusb_device_handle *dev)
{
	int ret;
	char response[LXC_MAX_IN_PACKET_SIZE];
	//unsigned char request[] = {0x0c, 0xb1, 0x01, 0x56, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x32, 0x05, 0x00, 0x00, 0x00, 0x00};
	//unsigned char request[] = {0x06, 0xb1, 0x01, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00};
	//unsigned char request[] = {0x06, 0xb1, 0x01, 0x0c, 0x64, 0x00, 0x00, 0x00, 0x00};

	//unsigned char request[] = {0x06, 0xb1, 0x01, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};
	//unsigned char request[] = {0x06, 0xb1, 0x01, 0x0e, 0x01, 0x00, 0x00, 0x00, 0x00};
	
	// STOP DIM
	//char request[] = {0x06, 0xb1, 0x01, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	//unsigned char request[] = {0x06, 0xb1, 0x01, 0x0e, 0x01, 0x00, 0x00, 0x00, 0x00};
	//unsigned char request[] = {0x06, 0xb1, 0x01, 0x0c, 0x60, 0x00, 0x00, 0x00, 0x00};
	//unsigned char request[] = {0x06, 0xb1, 0x02, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	char request[] = {0x06, 0xb1, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00};
	//request[4] = value;
	int transferred;

	int out_len = 9;
	//int out_len = 16;

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_OUT, (unsigned char *)request, out_len, &transferred, LXC_SEND_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt write error %d\n", ret);
		return;
	}
	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, (unsigned char *)response, LXC_MAX_IN_PACKET_SIZE, &transferred, LXC_RECV_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt read error %d\n", ret);
		return;
	}

	if (transferred < LXC_MAX_IN_PACKET_SIZE) {
		fprintf(stderr, "Short read %d\n", transferred);
	}

	hexdump(response, transferred);
}

void lxc_set_dim_level(struct libusb_device_handle *dev, uint8_t datapoint, uint8_t level) {
	union lxc_packet cmd;
	char response[LXC_MAX_IN_PACKET_SIZE];
	int ret;
	int transferred;
	int out_len = sizeof(cmd.data);

	init_packet(&cmd);
	cmd.cmd.datapoint = datapoint;
	cmd.cmd.opcode = LXC_OPCODE_DIM_SET;
	cmd.cmd.value = level;

	fprintf(stderr, "out: "); hexdump((char *)cmd.data, out_len);

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_OUT, cmd.data, out_len, &transferred, LXC_SEND_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt write error %d\n", ret);
		return;
	}
	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, (unsigned char *)response, LXC_MAX_IN_PACKET_SIZE, &transferred, LXC_RECV_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt read error %d\n", ret);
		return;
	}
	if (transferred < LXC_MAX_IN_PACKET_SIZE) {
		fprintf(stderr, "Short read %d\n", transferred);
	}

	fprintf(stderr, "in:  "); hexdump(response, transferred);
}

int lxc_get_dim_level(struct libusb_device_handle *dev, uint8_t datapoint) {
	union lxc_packet cmd;
	char response[LXC_MAX_IN_PACKET_SIZE];
	int ret;
	int transferred;
	int out_len = sizeof(cmd.data);

	init_packet(&cmd);
	cmd.cmd.datapoint = datapoint;
	cmd.cmd.opcode = LXC_OPCODE_DIM_GET;

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_OUT, (unsigned char *)cmd.data, out_len, &transferred, LXC_SEND_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt write error %d\n", ret);
		return -1;
	}
	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, (unsigned char *)response, LXC_MAX_IN_PACKET_SIZE, &transferred, LXC_RECV_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt read error %d\n", ret);
		return -1;
	}
	if (transferred < LXC_MAX_IN_PACKET_SIZE) {
		fprintf(stderr, "Short read %d\n", transferred);
	}

	hexdump(response, transferred);

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, (unsigned char *)response, LXC_MAX_IN_PACKET_SIZE, &transferred, LXC_RECV_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt read error %d\n", ret);
		return -1;
	}
	if (transferred < LXC_MAX_IN_PACKET_SIZE) {
		fprintf(stderr, "Short read %d\n", transferred);
		return -1;
	}

	fprintf(stderr, "in:  "); hexdump(response, transferred);

	if (response[5] > 100)
		return -1;

	return response[5];
}

void lxc_dim_gradual_start(struct libusb_device_handle *dev, uint8_t datapoint, int direction) {
	union lxc_packet cmd;
	char response[LXC_MAX_IN_PACKET_SIZE];
	int ret;
	int transferred;
	int out_len = sizeof(cmd.data);

	init_packet(&cmd);
	cmd.cmd.datapoint = datapoint;
	cmd.cmd.opcode = LXC_OPCODE_DIM_GRADUAL_START;
	cmd.cmd.value = direction;

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_OUT, (unsigned char *)cmd.data, out_len, &transferred, LXC_SEND_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt write error %d\n", ret);
		return;
	}
	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, (unsigned char *)response, LXC_MAX_IN_PACKET_SIZE, &transferred, LXC_RECV_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt read error %d\n", ret);
		return;
	}
	if (transferred < LXC_MAX_IN_PACKET_SIZE) {
		fprintf(stderr, "Short read %d\n", transferred);
	}

	fprintf(stderr, "in:  "); hexdump(response, transferred);
}

void lxc_dim_gradual_stop(struct libusb_device_handle *dev, uint8_t datapoint) {
	union lxc_packet cmd;
	char response[LXC_MAX_IN_PACKET_SIZE];
	int ret;
	int transferred;
	int out_len = sizeof(cmd.data);

	init_packet(&cmd);
	cmd.cmd.datapoint = datapoint;
	cmd.cmd.opcode = LXC_OPCODE_DIM_GRADUAL_STOP;

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_OUT, (unsigned char *)cmd.data, out_len, &transferred, LXC_SEND_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt write error %d\n", ret);
		return;
	}
	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, (unsigned char *)response, LXC_MAX_IN_PACKET_SIZE, &transferred, LXC_RECV_TIMEOUT);
	if (ret < 0) {
		fprintf(stderr, "Interrupt read error %d\n", ret);
		return;
	}
	if (transferred < LXC_MAX_IN_PACKET_SIZE) {
		fprintf(stderr, "Short read %d\n", transferred);
		return;
	}

	fprintf(stderr, "in:  "); hexdump(response, transferred);
}
