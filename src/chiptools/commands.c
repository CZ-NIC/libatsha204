#include <stdio.h>

#include "commands.h"
#include "../libatsha204/atsha204.h"

void dump_config(struct atsha_handle *handle)
{
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

void dump_data(struct atsha_handle *handle)
{
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

void dump_otp(struct atsha_handle *handle)
{
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

