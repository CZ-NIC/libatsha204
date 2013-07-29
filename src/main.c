#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "atsha204.h"
#include "main.h"
#include "communication.h"
#include "tools.h"
#include "operations.h"

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

int atsha_dev_rev(uint32_t *revision) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(g_config.device_fd);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_dev_rev();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(g_config.device_fd, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	*revision = op_dev_rev_recv(answer);

	//Let device sleep
	status = idle(g_config.device_fd);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_random(atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(g_config.device_fd);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_random();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(g_config.device_fd, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_random_recv(answer, &(number->data));
	if (number == 0) {
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(g_config.device_fd);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_read(unsigned char slot_number, atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(g_config.device_fd);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_read(slot_number);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(g_config.device_fd, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_read_recv(answer, &(number->data));
	if (number == 0) {
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(g_config.device_fd);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_write(unsigned char slot_number, atsha_big_int number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(g_config.device_fd);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_write(slot_number, number.bytes, number.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(g_config.device_fd, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_write_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(g_config.device_fd);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}


/*
 * From this point bellow is code just for testing and it simulation of
 * some application implementing this library.
 */

void testing_log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

int write_random_and_read() {
	unsigned char SLOT_ID = 8;
	int status;
	atsha_big_int number;

	fprintf(stderr, "Generate random number, write to and read again from slot %d\n", SLOT_ID);
	status = atsha_random(&number);
	fprintf(stderr, "Generate random number status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Generated %zu bytes random number: \n", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	//Write to slot
	status = atsha_write(SLOT_ID, number);
	fprintf(stderr, "Write to slot %d status: %s\n", SLOT_ID, atsha_error_name(status));
	free(number.data);

	// Read slot
	status = atsha_read(SLOT_ID, &number);
	fprintf(stderr, "Read from slot %d status: %s\n", SLOT_ID, atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: \n", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
		free(number.data);
	}

	return status;
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
	uint32_t rev = 0;
	status = atsha_dev_rev(&rev);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	fprintf(stderr, "Revision: %u\n", rev);

	// Random number
	atsha_big_int number;
	status = atsha_random(&number);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "%zu bytes number: ", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
		free(number.data);
	}

	// Read slot
	atsha_big_int number2;
	status = atsha_read(8, &number2);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: ", number2.bytes); for (size_t i = 0; i < number2.bytes; i++) { printf("%02X ", number2.data[i]); } printf("\n");
		free(number2.data);
	}

	write_random_and_read();

	close(g_config.device_fd);

	return 0;
}
