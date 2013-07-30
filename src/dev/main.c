#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "../libatsha204/atsha204.h"

void testing_log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

int write_random_and_read(struct atsha_handle *handle) {
	unsigned char SLOT_ID = 8;
	int status;
	atsha_big_int number;

	fprintf(stderr, "Generate random number, write to and read again from slot %d\n", SLOT_ID);
	status = atsha_random(handle, &number);
	fprintf(stderr, "Generate random number status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Generated %zu bytes random number: \n", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	//Write to slot
	status = atsha_slot_write(handle, SLOT_ID, number);
	fprintf(stderr, "Write to slot %d status: %s\n", SLOT_ID, atsha_error_name(status));
	free(number.data);

	// Read slot
	status = atsha_slot_read(handle, SLOT_ID, &number);
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

	struct atsha_handle *handle = atsha_open_usb_dev(argv[1]);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	atsha_set_verbose();
	atsha_set_log_callback(testing_log_callback);

	int status;

	// Get Revision
	fprintf(stderr, "Get revision:\n");
	uint32_t rev = 0;
	status = atsha_dev_rev(handle, &rev);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	fprintf(stderr, "Revision: %u\n", rev);

	// Random number
	fprintf(stderr, "Random number:\n");
	atsha_big_int number;
	status = atsha_random(handle, &number);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "%zu bytes number: ", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
		free(number.data);
	}

	// Read slot
	fprintf(stderr, "Read slot:\n");
	atsha_big_int number2;
	status = atsha_slot_read(handle, 8, &number2);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: ", number2.bytes); for (size_t i = 0; i < number2.bytes; i++) { printf("%02X ", number2.data[i]); } printf("\n");
		free(number2.data);
	}

	write_random_and_read(handle);

	fprintf(stderr, "Config read:\n");
	uint16_t config_word = 0;
	status = atsha_slot_conf_read(handle, 8, &config_word);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	fprintf(stderr, "Slot config word: %04X\n", config_word);

	atsha_close(handle);

	return 0;
}
