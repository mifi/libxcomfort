#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <MQTTClient.h>
#include <string.h>
#include <jansson.h>

#include "libxcomfort.h"

int main(int argc, const char **argv) {
	libusb_device_handle *lxc_dev;
	lxc_init();
        lxc_dev = lxc_open();

	if (!lxc_dev) {
		exit(EXIT_FAILURE);
	}

	if (argc != 4) {
		fprintf(stderr, "Usage %s [MQTT conn string] [MQTT user] [MQTT passwd]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	MQTTClient client;
	char *clientId = "xcomfort";
	char *connString = (char *)argv[1];

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = (char *)argv[2];
	conn_opts.password = (char *)argv[3];

	if (MQTTClient_create(&client, connString, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS) {
		perror("MQTTClient_create");
		exit(EXIT_FAILURE);
	}

	if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
		perror("MQTTClient_connect");
		exit(EXIT_FAILURE);
	}

	if (MQTTClient_subscribe(client, "xcomfort/#", 0) != MQTTCLIENT_SUCCESS) {
		perror("MQTTClient_subscribe");
		exit(EXIT_FAILURE);
	}


	while(1) {
		int topicLen;
		char *topicName = NULL;
		MQTTClient_message * message = NULL;


		json_t *root;
		json_error_t error;


		int response = MQTTClient_receive(client, &topicName, &topicLen, &message, 2000);

		if (response == MQTTCLIENT_SUCCESS || response == MQTTCLIENT_TOPICNAME_TRUNCATED) {
			if (message == NULL)
				continue;

			root = json_loads(message->payload, 0, &error); // TODO what if no \0

			fprintf(stderr, "%.*s\n", message->payloadlen, ((char *)message->payload));

			if (strcmp(topicName, "xcomfort/dim/level/set") == 0) {
				int datapoint = json_integer_value(json_object_get(root, "datapoint"));
				int level = json_integer_value(json_object_get(root, "level"));

				fprintf(stderr, "%d %d\n", datapoint, level);
				lxc_set_dim_level(lxc_dev, datapoint, level);
			}
			else if (strcmp(topicName, "xcomfort/dim/down") == 0) {
				int datapoint = json_integer_value(json_object_get(root, "datapoint"));

				fprintf(stderr, "%d down\n", datapoint);
				lxc_dim_gradual_start(lxc_dev, datapoint, 0);
			}
			else if (strcmp(topicName, "xcomfort/dim/up") == 0) {
				int datapoint = json_integer_value(json_object_get(root, "datapoint"));

				fprintf(stderr, "%d up\n", datapoint);
				lxc_dim_gradual_start(lxc_dev, datapoint, 1);
			}
			else if (strcmp(topicName, "xcomfort/dim/stop") == 0) {
				int datapoint = json_integer_value(json_object_get(root, "datapoint"));

				fprintf(stderr, "%d stop\n", datapoint);
				lxc_dim_gradual_stop(lxc_dev, datapoint);
			}
			else if (strcmp(topicName, "xcomfort/dim/level/get") == 0) {
				int datapoint = json_integer_value(json_object_get(root, "datapoint"));

				int dim_level = lxc_get_dim_level(lxc_dev, datapoint);
				fprintf(stderr, "TODO: dim level: %d\n", dim_level);
			}
			MQTTClient_free(topicName);
			MQTTClient_freeMessage(&message);
			json_decref(root);
		}
		else {
			fprintf(stderr, "response: %d\n", response);
			sleep(1);
		}
	}

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	lxc_close(lxc_dev);
	lxc_exit();
}
