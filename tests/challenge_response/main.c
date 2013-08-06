#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "../../src/libatsha204/atsha204.h"

void testing_log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hidraw device path\n", argv[0]);
		return 1;
	}

	struct atsha_handle *handle_usb = atsha_open_usb_dev(argv[1]);
	if (handle_usb == NULL) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	struct atsha_handle *handle_emul = atsha_open_emulation("atsha204.sw");
	if (handle_emul == NULL) {
		fprintf(stderr, "Couldn't open config.\n");
		return 1;
	}

	atsha_set_verbose();
	atsha_set_log_callback(testing_log_callback);

	atsha_big_int challenge = {
		.bytes = 32,
		.data = {
			0x79, 0x55, 0x98, 0x14, 0x78, 0x0F, 0xCC, 0xAA,
			0x09, 0x2C, 0xFA, 0xFA, 0xF8, 0x03, 0xF5, 0x18,
			0xA1, 0x3E, 0xC7, 0x50, 0x44, 0x44, 0x88, 0xF6,
			0x4D, 0xAC, 0xC2, 0x0B, 0x2A, 0xA3, 0x24, 0x5B
		}
	};

	atsha_big_int key = {
		.bytes = 32,
		.data = {
			0xF0, 0x43, 0xE1, 0xAE, 0x01, 0xEE, 0x13, 0x89,
			0x4B, 0x57, 0xBE, 0xBC, 0xA7, 0x4C, 0x04, 0x0E,
			0xC1, 0xC6, 0xE7, 0xF5, 0x34, 0x9B, 0x95, 0x78,
			0x74, 0xC3, 0x39, 0x7A, 0x33, 0x29, 0xFC, 0xF4
		}
	};

	//Store key to device - this is the same key
	if (atsha_slot_write(handle_usb, key) != ATSHA_ERR_OK) return 1;

	atsha_big_int response_usb;
	atsha_big_int response_emul;

	if (atsha_serial_number(handle_usb, &response_usb) != ATSHA_ERR_OK) return 1;
	if (atsha_serial_number(handle_emul, &response_emul) != ATSHA_ERR_OK) return 1;

	printf("SN HW: ");
	for (size_t i = 0; i < response_usb.bytes; i++) {
		printf("%02X ", response_usb.data[i]);
	}
	printf("\n");

	printf("SN SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");

	if (atsha_slot_read(handle_usb, &response_usb) != ATSHA_ERR_OK) return 1;
	if (atsha_slot_read(handle_emul, &response_emul) != ATSHA_ERR_OK) return 1;

	printf("KEY HW: ");
	for (size_t i = 0; i < response_usb.bytes; i++) {
		printf("%02X ", response_usb.data[i]);
	}
	printf("\n");

	printf("KEY SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");

	if (atsha_low_challenge_response(handle_usb, atsha_find_slot_number(handle_usb), challenge, &response_usb, true) != ATSHA_ERR_OK) return 1;
	if (atsha_low_challenge_response(handle_emul, atsha_find_slot_number(handle_emul), challenge, &response_emul, true) != ATSHA_ERR_OK) return 1;

	printf("HMAC HW: ");
	for (size_t i = 0; i < response_usb.bytes; i++) {
		printf("%02X ", response_usb.data[i]);
	}
	printf("\n");

	printf("HMAC SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");

	if (atsha_low_challenge_response(handle_usb, atsha_find_slot_number(handle_usb), challenge, &response_usb, false) != ATSHA_ERR_OK) return 1;
	if (atsha_low_challenge_response(handle_emul, atsha_find_slot_number(handle_emul), challenge, &response_emul, false) != ATSHA_ERR_OK) return 1;

	printf("HMAC HW: ");
	for (size_t i = 0; i < response_usb.bytes; i++) {
		printf("%02X ", response_usb.data[i]);
	}
	printf("\n");

	printf("HMAC SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");

	if (atsha_low_challenge_response_mac(handle_usb, atsha_find_slot_number(handle_usb), challenge, &response_usb, true) != ATSHA_ERR_OK) return 1;
	if (atsha_low_challenge_response_mac(handle_emul, atsha_find_slot_number(handle_emul), challenge, &response_emul, true) != ATSHA_ERR_OK) return 1;

	printf("MAC HW: ");
	for (size_t i = 0; i < response_usb.bytes; i++) {
		printf("%02X ", response_usb.data[i]);
	}
	printf("\n");

	printf("MAC SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");

	if (atsha_low_challenge_response_mac(handle_usb, atsha_find_slot_number(handle_usb), challenge, &response_usb, false) != ATSHA_ERR_OK) return 1;
	if (atsha_low_challenge_response_mac(handle_emul, atsha_find_slot_number(handle_emul), challenge, &response_emul, false) != ATSHA_ERR_OK) return 1;

	printf("MAC HW: ");
	for (size_t i = 0; i < response_usb.bytes; i++) {
		printf("%02X ", response_usb.data[i]);
	}
	printf("\n");

	printf("MAC SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");

	atsha_close(handle_usb);
	atsha_close(handle_emul);

	return 0;
}
