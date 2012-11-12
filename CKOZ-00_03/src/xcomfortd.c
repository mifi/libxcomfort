#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <microhttpd.h>

#include "libxcomfort.h"

libusb_device_handle *lxc_dev;


static int return_status_code(struct MHD_Connection *connection, unsigned int status_code) {
	struct MHD_Response *response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
	int ret = MHD_queue_response(connection, status_code, response);
	MHD_destroy_response(response);
	return ret;
}

static int callback(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
	static int aptr;
	struct MHD_Response *response;
	int ret;
	const char *datapoint;
	const char *direction;
	
	int dim_level;
	char *dim_level_str = "";

	if (&aptr != *ptr) {
		/* do not respond on first call */
		*ptr = &aptr;
		return MHD_YES;
	}

	*ptr = NULL; /* reset when done */
	
	if (strcmp(url, "/dim_gradual_start") == 0) {
		datapoint = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "datapoint");
		direction = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "direction");
		
		if (datapoint == NULL || direction == NULL) {
			return MHD_NO;
		}

		lxc_dim_gradual_start(lxc_dev, (uint8_t)atoi(datapoint), atoi(direction));

		return return_status_code(connection, MHD_HTTP_OK);
	}
	else if (strcmp(url, "/dim_gradual_stop") == 0) {
		datapoint = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "datapoint");
		
		if (datapoint == NULL) {
			return MHD_NO;
		}

		lxc_dim_gradual_stop(lxc_dev, (uint8_t)atoi(datapoint));

		// Get dim level also
		dim_level = lxc_get_dim_level(lxc_dev, (uint8_t)atoi(datapoint));
		asprintf(&dim_level_str, "%d", dim_level);

		response = MHD_create_response_from_buffer(strlen(dim_level_str), (void *)dim_level_str, MHD_RESPMEM_MUST_FREE);
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		MHD_destroy_response(response);
		return ret;
	}
	else if (strcmp(url, "/get_dim_level") == 0) {
		datapoint = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "datapoint");

		if (datapoint == NULL) {
			return MHD_NO;
		}

		dim_level = lxc_get_dim_level(lxc_dev, (uint8_t)atoi(datapoint));
		asprintf(&dim_level_str, "%d", dim_level);

		response = MHD_create_response_from_buffer(strlen(dim_level_str), (void *)dim_level_str, MHD_RESPMEM_MUST_FREE);
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		MHD_destroy_response(response);
		return ret;
	}
	else if (strcmp(url, "/set_dim_level") == 0) {
		datapoint = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "datapoint");
		dim_level_str = (char *)MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "level");

		if (datapoint == NULL || dim_level_str == NULL) {
			return MHD_NO;
		}

		lxc_set_dim_level(lxc_dev, (uint8_t)atoi(datapoint), (uint8_t)atoi(dim_level_str));

		return return_status_code(connection, MHD_HTTP_OK);
	}
	else {
		return MHD_NO;
	}
}

int main(int argc, char *const *argv) {
	struct MHD_Daemon *daemon;

	if (argc != 2) {
		printf ("%s PORT\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	lxc_init();
	lxc_dev = lxc_open();

	if (!lxc_dev) {
		exit(EXIT_FAILURE);
	}

	daemon = MHD_start_daemon (
		MHD_USE_SELECT_INTERNALLY,
		atoi(argv[1]),
		NULL, NULL, &callback, NULL,
		MHD_OPTION_CONNECTION_LIMIT, 1,
		MHD_OPTION_END);

	if (daemon == NULL) {
		fprintf(stderr, "Failed to start server\n");
		lxc_close(lxc_dev);
		lxc_exit();
		exit(EXIT_FAILURE);
	}
	
	fgetc(stdin);

	lxc_close(lxc_dev);
	lxc_exit();
	MHD_stop_daemon (daemon);
	return 0;
}
