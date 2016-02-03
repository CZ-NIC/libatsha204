/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2016 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include "commands.h"
#include "../libatsha204/atsha204.h"

bool dump_config(struct atsha_handle *handle)
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
			return false;
		}
	}
	printf("\n");

	return true;
}

bool dump_data(struct atsha_handle *handle)
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
			return false;
		}
	}
	printf("\n");

	return true;
}

bool dump_otp(struct atsha_handle *handle)
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
			return false;
		}
	}
	printf("\n");

	return true;
}

