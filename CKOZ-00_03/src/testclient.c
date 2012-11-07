#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#include "libxcomfort.h"

int main(int argc, char *argv[])
{
	int datapoint = 1;
	libusb_device_handle *dev;

	lxc_init();
	dev = lxc_open();

	if (!dev) {
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "level %d\n", lxc_get_dim_level(dev, datapoint));

	lxc_dim_gradual_start(dev, datapoint, 1);
	usleep(100000);
	lxc_dim_gradual_stop(dev, datapoint);

	fprintf(stderr, "level %d\n", lxc_get_dim_level(dev, datapoint));

	lxc_close(dev);
	lxc_exit();

	return 0;
}
