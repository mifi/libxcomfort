#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

	MQTTClient client;
	char *clientId = "xcomfort";
	char *connString = "tcp://vps.mifi.no:1883";

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

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

			if (strcmp(topicName, "xcomfort/setDimLevel") == 0) {
				int datapoint = json_integer_value(json_object_get(root, "datapoint"));
				int level = json_integer_value(json_object_get(root, "level"));

				fprintf(stderr, "%d %d\n", datapoint, level);
				lxc_set_dim_level(lxc_dev, datapoint, level);
			}
			else if (strcmp(topicName, "xcomfort/getDimLevel") == 0) {
				/*dim_level = lxc_get_dim_level(lxc_dev, (uint8_t)atoi(datapoint));
				//asprintf(&dim_level_str, "%d", dim_level);*/
			}
			MQTTClient_free(topicName);
			MQTTClient_freeMessage(&message);
			json_decref(root);
		}
		else {
			fprintf(stderr, "response: %d\n", response);
		}
	}

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	lxc_close(lxc_dev);
	lxc_exit();
}
