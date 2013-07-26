#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h> //strerror()
#include<stdint.h>
#include<stdbool.h>

#include "main.h"
#include "error.h"
#include "communication.h"
#include "tools.h"
#include "operations.h"

/**
 * Struct for global configuration of library
 */
typedef struct {
	int device_fd;
	bool verbose;
	void (*log_callback)(const char* msg);
} atsha_configuration;

/**
 * Global variable with configuration and some initial config values.
 */
atsha_configuration g_config = {
	.device_fd = 0,
	.verbose = false,
	.log_callback = NULL
};

static const char *WARNING_WAKE_NOT_CONFIRMED = "Device is possibly still awake";

void log_message(const char* msg) {
	if (g_config.log_callback != NULL) {
		g_config.log_callback(msg);
	}
}

void atsha_set_verbose() {
	g_config.verbose = true;
}

void atsha_set_log_callback(void (*clb)(const char* msg)) {
	g_config.log_callback = clb;
}

int dev_rev(uint32_t *revision) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(g_config.device_fd);
	if (status != ERR_OK) return status;

	packet = op_dev_rev();
	if (!packet) return ERR_MEMORY_ALLOCATION_ERROR;

	status = command(g_config.device_fd, packet, &answer, true);
	if (status != ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	*revision = op_dev_rev_recv(answer);

	//Let device sleep
	status = idle(g_config.device_fd);
	if (status != ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ERR_OK;
}

int chl_random(big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(g_config.device_fd);
	if (status != ERR_OK) return status;

	packet = op_random();
	if (!packet) return ERR_MEMORY_ALLOCATION_ERROR;

	status = command(g_config.device_fd, packet, &answer, false);
	if (status != ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_random_recv(answer, &(number->data));
	if (number == 0) {
		return ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(g_config.device_fd);
	if (status != ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ERR_OK;
}

/*
 * From this point bellow is code just for testing and it simulation of
 * some application implementing this library.
 */

void testing_log_callback(const char *msg) {
	fprintf(stderr, "Warning: %s\n", msg);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hidraw device path\n", argv[0]);
		return 1;
	}

	g_config.device_fd = open(argv[1], O_RDWR);
	if (g_config.device_fd == -1) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	atsha_set_verbose();
	atsha_set_log_callback(testing_log_callback);

	int status;

	// Get Revision
	uint32_t rev;
	status = dev_rev(&rev);
	fprintf(stderr, "Status: %s\n", error_name(status));
	fprintf(stderr, "Revision: %u\n", rev);

	// Random number
	big_int number;
	status = chl_random(&number);
	fprintf(stderr, "Status: %s\n", error_name(status));
	fprintf(stderr, "%zu bytes number: ", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	free(number.data);

	close(g_config.device_fd);

	return 0;
}
