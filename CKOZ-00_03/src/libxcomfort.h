#ifndef LIBXCOMFORT_H
#define LIBXCOMFORT_H

#include <libusb-1.0/libusb.h>

#define LXC_MAX_OUT_PACKET_SIZE 9
#define LXC_MAX_IN_PACKET_SIZE 19

#define LXC_SEND_TIMEOUT 1000
#define LXC_RECV_TIMEOUT 1000

#define LXC_USB_VENDOR 0x188a // Moeller
#define LXC_USB_PRODUCT 0x1101 // USB gateway interface
#define LXC_USB_INTERFACE 0
#define LXC_USB_ENDPOINT_IN 0x81 // EP 1 IN
#define LXC_USB_ENDPOINT_OUT 0x02 // EP 2 OUT

enum lxc_pkt_type {
	LXC_PKT_TYPE_OUT = 0xb1,
	LXC_PKT_TYPE_IN = 0xc1,
	LXC_PKT_TYPE_ACK = 0xc3,
};

enum lxc_opcode {
	LXC_OPCODE_ON_OFF = 0x0a, // value = 0 means off, 1 means on
	LXC_OPCODE_DIM_GET = 0x0b, // Get current value from actuator
	LXC_OPCODE_DIM_SET = 0x0c, // Percent value 0-100
	LXC_OPCODE_DIM_GRADUAL_STOP = 0x0d, // aka OFF BUTTON RELEASE (works for stopping both directions)
	LXC_OPCODE_DIM_GRADUAL_START = 0x0e, // aka OFF/ON BUTTON HOLD: Start gradual dim up or down, value = 0 is down/off, 1 is up/on
	LXC_OPCODE_ON_OFF2 = 0x0f, // seems identical to LXC_OPCODE_ON_OFF.

	LXC_OPCODE_SEND_MEASUREMENT_VALUE = 0x11,
	LXC_OPCODE_SEND_MEASUREMENT_VALUE2 = 0x1a,
	LXC_OPCODE_UNKNOWN1 = 0x2a,
	LXC_OPCODE_UNKNOWN2 = 0x2b,
	LXC_OPCODE_SEND_TEMPERATURE = 0x2c, // Some kind of temperature
	LXC_OPCODE_SEND_MEASUREMENT_VALUE3 = 0x30, // Has zero decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE4 = 0x31, // Has one decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE5 = 0x32, // Has two decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE6 = 0x33, // Has three decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE7 = 0x40, // Has zero decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE8 = 0x41, // Has one decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE9 = 0x42, // Has two decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE10 = 0x43, // Has three decimals?
	LXC_OPCODE_SEND_TEMPERATURE2 = 0x44,
	LXC_OPCODE_SEND_TEMPERATURE3 = 0x45,
};

union lxc_packet {
	uint8_t data[LXC_MAX_OUT_PACKET_SIZE];
	struct {
		uint8_t size; // Seems it should always be 6
		uint8_t type; // enum lxc_pkt_type
		uint8_t datapoint;
		uint8_t opcode; // enum lxc_opcode
		uint8_t value; // Used only sometimes, else 0
		uint8_t unused[4]; // Seems it should be all zero
	} cmd;
};

libusb_device_handle *lxc_open();
void lxc_close(libusb_device_handle *dev);

void lxc_init(void);
void lxc_exit(void);

void lxc_set_dim_level(struct libusb_device_handle *dev, uint8_t datapoint, uint8_t level);
int lxc_get_dim_level(struct libusb_device_handle *dev, uint8_t datapoint);
void lxc_dim_gradual_start(struct libusb_device_handle *dev, uint8_t datapoint, int direction);
void lxc_dim_gradual_stop(struct libusb_device_handle *dev, uint8_t datapoint);

#endif
