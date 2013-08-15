#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include<string.h>

#include "../libatsha204/atsha204.h"

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

static void dump_config(struct atsha_handle *handle) {
	atsha_big_int data;

	printf("Config zone (0x00 - 0x15):\n");
	for (unsigned char addr = 0x00; addr <= 0x15; addr++) {
		printf("0x%02X: ", addr);
		if (atsha_raw_conf_read(handle, addr, &data) == ATSHA_ERR_OK) {
			for (size_t i = 0; i < data.bytes; i++) {
				printf("%02X ", data.data[i]);
			}
			printf("\n");
		} else {
			printf("ERROR\n");
		}
	}
	printf("\n");
}

static void dump_data(struct atsha_handle *handle) {
	atsha_big_int data;

	printf("Data zone (slot 0 - 15):\n");
	for (unsigned char slot = 0; slot <= 15; slot++) {
		printf("%2u: ", slot);
		if (atsha_raw_slot_read(handle, slot, &data) == ATSHA_ERR_OK) {
			for (size_t i = 0; i < data.bytes; i++) {
				printf("%02X ", data.data[i]);
			}
			printf("\n");
		} else {
			printf("ERROR\n");
		}
	}
	printf("\n");
}

static void dump_otp(struct atsha_handle *handle) {
	atsha_big_int data;

	printf("OTP zone (0x00 - 0x0F):\n");
	for (unsigned char addr = 0x00; addr <= 0x0F; addr++) {
		printf("0x%02X: ", addr);
		if (atsha_raw_otp_read(handle, addr, &data) == ATSHA_ERR_OK) {
			for (size_t i = 0; i < data.bytes; i++) {
				printf("%02X ", data.data[i]);
			}
			printf("\n");
		} else {
			printf("ERROR\n");
		}
	}
	printf("\n");
}

static void print_abi(atsha_big_int abi) {
	for (size_t i = 0; i < abi.bytes; i++) {
		fprintf(stderr, "%02X ", abi.data[i]);
	}
	fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s command\n", argv[0]);
		return 1;
	}

	atsha_big_int abi;

	//init LIBATSHA204
	atsha_set_verbose();
	atsha_set_log_callback(log_callback);

	//Create LIBATSHA204 handler
	struct atsha_handle *handle = atsha_open_i2c_dev();
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open I2C devidce.\n");
		return 1;
	}
	char *cmd = argv[1];
	if (strcmp(cmd, "dump-config") == 0) {
		dump_config(handle);

	} else if (strcmp(cmd, "dump-otp") == 0) {
		dump_otp(handle);

	} else if (strcmp(cmd, "dump-data") == 0) {
		dump_data(handle);

	} else if (strcmp(cmd, "sn") == 0) {
		atsha_serial_number(handle, &abi);
		print_abi(abi);

	} else if (strcmp(cmd, "random") == 0) {
		atsha_random(handle, &abi);
		print_abi(abi);

	} else if (strcmp(cmd, "compiled") == 0) {
		if (atsha_raw_slot_read(handle, 0, &abi) == ATSHA_ERR_OK) {
			print_abi(abi);
		}

	} else {
		fprintf(stderr, "Undefined command\n");
	}

	atsha_close(handle);

	return 0;
}
