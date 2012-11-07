#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#include "libxcomfort.h"

#define ON_OFF_THRESHOLD 10


libusb_device_handle *dev;
int done = 0;


void sigint() {
	done = 1;
}

int read_target() {
	char buf[5+1];
	size_t n_read;
	int ret = -1;
	
	FILE *f = fopen("target_val", "r");
	
	if (f != NULL) {
		n_read = fread(buf, 1, 5, f);
		
		if (n_read >= 1) {
			buf[n_read] = '\0';
			ret = atoi(buf);
			
			if (ret > 100 || ret < 0) {
				ret = -1;
			}
		}
		fclose(f);
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int target_val;
	int target_val_old = -1;
	int dir;
	int diff;

	if (argc == 3) {
		target_val = atoi(argv[1]);
		dir = strcmp(argv[2], "+") == 0 ? 1 : 0;
		
		if (target_val < 0 || target_val > 100) {
			exit(EXIT_FAILURE);
		}
	}

	signal(SIGINT, sigint);

	lxc_init();
	dev = lxc_open();
	
	if (!dev) {
		exit(EXIT_FAILURE);
	}

	int datapoint = 1;
	/*struct timeval start, stop;
	double diff_time;
	int ms_to_sleep;

	while (!done) {
		target_val = read_target();

		if (target_val == target_val_old) {
			usleep(100000);
			continue;
		}
		target_val_old = target_val;

		int current = lxc_get_dim_level(dev, datapoint);
		fprintf(stderr, "LEVEL: %d, TARGET: %d\n", current, target_val);
		if (current < 0 || target_val < 0) {
			sleep(100000);
			continue;
		}

		diff = target_val - current;

		if (abs(diff) >= ON_OFF_THRESHOLD || target_val >= 100-ON_OFF_THRESHOLD || target_val <= ON_OFF_THRESHOLD) {
			if (target_val >= 100-ON_OFF_THRESHOLD) {
				dir = 1;
			}
			else if (target_val <= ON_OFF_THRESHOLD) {
				dir = 0;
			}
			else {
				dir = diff > 0 ? 1 : 0;
			}

			gettimeofday(&start, NULL);
			lxc_dim_gradual_start(dev, datapoint, dir);
			gettimeofday(&stop, NULL);

			diff_time = ((double)stop.tv_sec + stop.tv_usec / 1000000.0) - ((double)start.tv_sec + start.tv_usec / 1000000.0);
			fprintf(stderr, "%f\n", diff_time);

			ms_to_sleep = 100 + (6*1000/100)*abs(diff) - (int)(diff_time*1000);

			if (ms_to_sleep > 0) {
				usleep(ms_to_sleep * 1000);
			}
			fprintf(stderr, "slept %d msec\n", ms_to_sleep);
			lxc_dim_gradual_stop(dev, datapoint);

			current = lxc_get_dim_level(dev, datapoint);
			fprintf(stderr, "LEVEL: %d\n", current);
			
			// Make sure we hit boundry values
			if (target_val >= 100-ON_OFF_THRESHOLD) {
				lxc_set_dim_level(dev, datapoint, 100);
			}
			else if (target_val <= ON_OFF_THRESHOLD) {
				lxc_set_dim_level(dev, datapoint, 0);
			}
		}
	}*/

	fprintf(stderr, "level %d\n", lxc_get_dim_level(dev, 1));

	lxc_close(dev);
	lxc_exit();

	return 0;
}
