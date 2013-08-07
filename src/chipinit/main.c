#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "../libatsha204/atsha204.h"

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hidraw device path\n", argv[0]);
		return 1;
	}

	atsha_set_verbose();
	atsha_set_log_callback(log_callback);

	struct atsha_handle *handle = atsha_open_usb_dev(argv[1]);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	atsha_big_int data;
/*
	data.bytes=4;
	data.data[0] = 0x00;
	data.data[1] = 0x11;
	data.data[2] = 0x22;
	data.data[3] = 0x33;

	int status;
	status = atsha_raw_otp_write(handle, 0x00, data);
	printf("Status: %s\n", atsha_error_name(status));
	sleep(2);
	atsha_raw_otp_read(handle, 0x00, &data);
	for (size_t i = 0; i < data.bytes; i++) {
		printf("%02X ", data.data[i]);
	}

	atsha_close(handle);
	return 0;
*/
	printf("OTP zone (0x00 - 0x0F):\n");
	for (unsigned char addr = 0x00; addr <= 0x0F; addr++) {
		sleep(2);
		atsha_raw_otp_read(handle, addr, &data);
		printf("0x%02X: ", addr);
		for (size_t i = 0; i < data.bytes; i++) {
			printf("%02X ", data.data[i]);
		}
		printf("\n");
	}
	printf("\n");

	printf("Config zone (0x00 - 0x15):\n");
	for (unsigned char addr = 0x00; addr <= 0x15; addr++) {
		sleep(2);
		atsha_raw_conf_read(handle, addr, &data);
		printf("0x%02X: ", addr);
		for (size_t i = 0; i < data.bytes; i++) {
			printf("%02X ", data.data[i]);
		}
		printf("\n");
	}
	printf("\n");

	printf("Data zone (slot 0 - 15):\n");
	for (unsigned char slot = 0; slot <= 15; slot++) {
		sleep(2);
		atsha_low_slot_read(handle, slot, &data);
		printf("%2u: ", slot);
		for (size_t i = 0; i < data.bytes; i++) {
			printf("%02X ", data.data[i]);
		}
		printf("\n");
	}

	atsha_close(handle);

	return 0;
}
