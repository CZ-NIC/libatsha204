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
#include <stdint.h>
#include <stdbool.h>

#include "../../src/libatsha204/atsha204.h"

static bool cmp_responses(atsha_big_int r1, atsha_big_int r2) {
	if (r1.bytes != r2.bytes) return false;

	for (size_t i = 0; i < r1.bytes; i++) {
		if (r1.data[i] != r2.data[i]) return false;
	}

	return true;
}

bool test(struct atsha_handle *handle, const char *filename)
{
	struct atsha_handle *handle_emul = atsha_open_emulation(filename);
	if (handle_emul == NULL) {
		fprintf(stderr, "Couldn't open emulation handler.\n");
		return false;
	}

	atsha_big_int challenge = {
		.bytes = 32,
		.data = {
			0x79, 0x55, 0x98, 0x14, 0x78, 0x0F, 0xCC, 0xAA,
			0x09, 0x2C, 0xFA, 0xFA, 0xF8, 0x03, 0xF5, 0x18,
			0xA1, 0x3E, 0xC7, 0x50, 0x44, 0x44, 0x88, 0xF6,
			0x4D, 0xAC, 0xC2, 0x0B, 0x2A, 0xA3, 0x24, 0x5B
		}
	};

	atsha_big_int response_i2c;
	atsha_big_int response_emul;

	if (atsha_serial_number(handle, &response_i2c) != ATSHA_ERR_OK) return false;
	if (atsha_serial_number(handle_emul, &response_emul) != ATSHA_ERR_OK) return false;

	printf("SN HW: ");
	for (size_t i = 0; i < response_i2c.bytes; i++) {
		printf("%02X ", response_i2c.data[i]);
	}
	printf("\n");

	printf("SN SW: ");
	for (size_t i = 0; i < response_emul.bytes; i++) {
		printf("%02X ", response_emul.data[i]);
	}
	printf("\n");
	if (!cmp_responses(response_i2c, response_emul)) return false;

	for (unsigned char slot = 0; slot < 16; slot++) {
		printf("================================================== %02u ==================================================\n", slot);
		if (atsha_low_challenge_response(handle, slot, challenge, &response_i2c, true) != ATSHA_ERR_OK) return false;
		if (atsha_low_challenge_response(handle_emul, slot, challenge, &response_emul, true) != ATSHA_ERR_OK) return false;

		printf("HMAC HW: ");
		for (size_t i = 0; i < response_i2c.bytes; i++) {
			printf("%02X ", response_i2c.data[i]);
		}
		printf("\n");

		printf("HMAC SW: ");
		for (size_t i = 0; i < response_emul.bytes; i++) {
			printf("%02X ", response_emul.data[i]);
		}
		printf("\n");
		if (!cmp_responses(response_i2c, response_emul)) return false;

		if (atsha_low_challenge_response(handle, slot, challenge, &response_i2c, false) != ATSHA_ERR_OK) return false;
		if (atsha_low_challenge_response(handle_emul, slot, challenge, &response_emul, false) != ATSHA_ERR_OK) return false;

		printf("HMAC HW: ");
		for (size_t i = 0; i < response_i2c.bytes; i++) {
			printf("%02X ", response_i2c.data[i]);
		}
		printf("\n");

		printf("HMAC SW: ");
		for (size_t i = 0; i < response_emul.bytes; i++) {
			printf("%02X ", response_emul.data[i]);
		}
		printf("\n");
		if (!cmp_responses(response_i2c, response_emul)) return false;

		if (atsha_low_challenge_response_mac(handle, slot, challenge, &response_i2c, true) != ATSHA_ERR_OK) return false;
		if (atsha_low_challenge_response_mac(handle_emul, slot, challenge, &response_emul, true) != ATSHA_ERR_OK) return false;

		printf("MAC  HW: ");
		for (size_t i = 0; i < response_i2c.bytes; i++) {
			printf("%02X ", response_i2c.data[i]);
		}
		printf("\n");

		printf("MAC  SW: ");
		for (size_t i = 0; i < response_emul.bytes; i++) {
			printf("%02X ", response_emul.data[i]);
		}
		printf("\n");
		if (!cmp_responses(response_i2c, response_emul)) return false;

		if (atsha_low_challenge_response_mac(handle, slot, challenge, &response_i2c, false) != ATSHA_ERR_OK) return false;
		if (atsha_low_challenge_response_mac(handle_emul, slot, challenge, &response_emul, false) != ATSHA_ERR_OK) return false;

		printf("MAC  HW: ");
		for (size_t i = 0; i < response_i2c.bytes; i++) {
			printf("%02X ", response_i2c.data[i]);
		}
		printf("\n");

		printf("MAC  SW: ");
		for (size_t i = 0; i < response_emul.bytes; i++) {
			printf("%02X ", response_emul.data[i]);
		}
		printf("\n");
		if (!cmp_responses(response_i2c, response_emul)) return false;
	}

	printf("================================================== OTP =================================================\n");

	for (unsigned char addr = 0x00; addr <= 0x0F; addr++) {
		if (atsha_raw_otp_read(handle, addr, &response_i2c) != ATSHA_ERR_OK) return false;
		if (atsha_raw_otp_read(handle_emul, addr, &response_emul) != ATSHA_ERR_OK) return false;

		printf("0x%02X: %02X %02X %02X %02X \t\t\t 0x%02X: %02X %02X %02X %02X\n",
			addr,
			response_i2c.data[0],
			response_i2c.data[1],
			response_i2c.data[2],
			response_i2c.data[3],
			addr,
			response_emul.data[0],
			response_emul.data[1],
			response_emul.data[2],
			response_emul.data[3]
		);
		if (!cmp_responses(response_i2c, response_emul)) return false;
	}

	printf("================================================== CNF =================================================\n");
	atsha_big_int data;

	for (unsigned char addr = 0x00; addr <= 0x15; addr++) {
		atsha_raw_conf_read(handle, addr, &data);
		printf("0x%02X: ", addr);
		for (size_t i = 0; i < data.bytes; i++) {
			printf("%02X ", data.data[i]);
		}
		printf("\n");
	}

	// Ensure that this handler is closed is not necessary
	// There aren't any lock, it is just open file and allocated memory
	atsha_close(handle_emul);

	return true;
}
