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
	int status;
	atsha_big_int number;

	fprintf(stderr, "Generate random number, write to and read again from slot\n");
	status = atsha_random(handle, &number);
	fprintf(stderr, "Generate random number status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Generated %zu bytes random number: \n", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	} else {
		return status;
	}

	//Write to slot
	status = atsha_slot_write(handle, number);
	fprintf(stderr, "Write to slot status: %s\n", atsha_error_name(status));

	// Read slot
	status = atsha_slot_read(handle, &number);
	fprintf(stderr, "Read from slot status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: \n", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	return status;
}

int hmac(struct atsha_handle *handle) {
	int status;
	atsha_big_int number;
	atsha_big_int digest;

	fprintf(stderr, "Generate HMAC digest with key from slot\n");
	status = atsha_random(handle, &number);
	fprintf(stderr, "Generate random number status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Generated %zu bytes random number: \n", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	status = atsha_challenge_response(handle, number, &digest);
	fprintf(stderr, "HMAC digest status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "HMAC D is %zu bytes number: \n", digest.bytes); for (size_t i = 0; i < digest.bytes; i++) { printf("%02X ", digest.data[i]); } printf("\n");
	}

	status = atsha_low_challenge_response(handle, atsha_find_slot_number(handle), number, &digest, true);
	fprintf(stderr, "HMAC digest status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "HMAC T is %zu bytes number: \n", digest.bytes); for (size_t i = 0; i < digest.bytes; i++) { printf("%02X ", digest.data[i]); } printf("\n");
	}

	status = atsha_low_challenge_response(handle, atsha_find_slot_number(handle), number, &digest, false);
	fprintf(stderr, "HMAC digest status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "HMAC F is %zu bytes number: \n", digest.bytes); for (size_t i = 0; i < digest.bytes; i++) { printf("%02X ", digest.data[i]); } printf("\n");
	}

	return status;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hidraw device path\n", argv[0]);
		return 1;
	}

	atsha_set_verbose();
	atsha_set_log_callback(testing_log_callback);

	//struct atsha_handle *handle = atsha_open_usb_dev(argv[1]);
	struct atsha_handle *handle = atsha_open_emulation("atsha204.sw");
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	int status;

	////////////////////////////////////////////////////////////////////
	int slot = atsha_find_slot_number(handle);
	if (slot == 255) {
		fprintf(stderr, "DNS communication error\n");
	} else {
		printf("Slot number is: %d\n", slot);
	}

	atsha_close(handle);

	return 0;
	////////////////////////////////////////////////////////////////////

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
	}

	// Read slot
	fprintf(stderr, "Read slot:\n");
	atsha_big_int number2;
	status = atsha_slot_read(handle, &number2);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: ", number2.bytes); for (size_t i = 0; i < number2.bytes; i++) { printf("%02X ", number2.data[i]); } printf("\n");
	}

	write_random_and_read(handle);

	hmac(handle);

	//SN
	fprintf(stderr, "Serial number:\n");
	atsha_big_int sn;
	status = atsha_serial_number(handle, &sn);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "SN contents %zu bytes number: ", sn.bytes); for (size_t i = 0; i < sn.bytes; i++) { printf("%02X ", sn.data[i]); } printf("\n");
	}

	atsha_close(handle);

	fprintf(stderr, "SW Emulation\n============================================================\n\n");

	handle = atsha_open_emulation("atsha204.sw");
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open config file.\n");
		return 1;
	}

	// Get Revision
	fprintf(stderr, "Get revision:\n");
	rev = 0;
	status = atsha_dev_rev(handle, &rev);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	fprintf(stderr, "Revision: %u\n", rev);

	// Random number
	fprintf(stderr, "Random number:\n");
	status = atsha_random(handle, &number);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "%zu bytes number: ", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	// Read slot
	fprintf(stderr, "Read slot:\n");
	status = atsha_slot_read(handle, &number2);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: ", number2.bytes); for (size_t i = 0; i < number2.bytes; i++) { printf("%02X ", number2.data[i]); } printf("\n");
	}

	write_random_and_read(handle);

	hmac(handle);

	//SN
	fprintf(stderr, "Serial number:\n");
	status = atsha_serial_number(handle, &sn);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "SN contents %zu bytes number: ", sn.bytes); for (size_t i = 0; i < sn.bytes; i++) { printf("%02X ", sn.data[i]); } printf("\n");
	}

	atsha_close(handle);

	fprintf(stderr, "SW Server Emulation\n============================================================\n\n");
	unsigned char ssekey[32] = {
		0xBE, 0xBA, 0x48, 0x40, 0xD6, 0x45, 0xDE, 0xE5,
		0xA2, 0x37, 0x6F, 0x2F, 0x68, 0x35, 0x3B, 0xA6,
		0xAA, 0x74, 0xE1, 0xA1, 0x3C, 0xE8, 0xB6, 0x99,
		0x95, 0xAB, 0x31, 0x83, 0x1A, 0x01, 0xFB, 0x9A
	};

	unsigned char ssesn[9] = {
		0x01, 0x23, 0x43, 0x39, 0xB5, 0x36, 0xA7, 0xC9, 0xEE
	};

	handle = atsha_open_server_emulation(ssesn, ssekey);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't create server emulation handler.\n");
		return 1;
	}

	fprintf(stderr, "Serial number:\n");
	status = atsha_serial_number(handle, &sn);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "SN contents %zu bytes number: ", sn.bytes); for (size_t i = 0; i < sn.bytes; i++) { printf("%02X ", sn.data[i]); } printf("\n");
	}

	// Read slot
	fprintf(stderr, "Read slot:\n");
	status = atsha_slot_read(handle, &number2);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Slot contents %zu bytes number: ", number2.bytes); for (size_t i = 0; i < number2.bytes; i++) { printf("%02X ", number2.data[i]); } printf("\n");
	}

	atsha_close(handle);

	return 0;
}
