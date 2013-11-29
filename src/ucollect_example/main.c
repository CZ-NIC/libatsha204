/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
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

#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "../libatsha204/atsha204.h"

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

void print_number(size_t len, unsigned char *data) {
	for (size_t i = 0; i < len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\n");
}

int main(int argc, char **argv) {
	(void) argc; (void) argv;
	//Obtain SN and key e.g. from DB server
	unsigned char sn_data[9] = {
		0x01, 0x23, 0x43, 0x39, 0xB5, 0x36, 0xA7, 0xC9, 0xEE
	};

	unsigned char key_data[32] = {
		0xBE, 0xBA, 0x48, 0x40, 0xD6, 0x45, 0xDE, 0xE5,
		0xA2, 0x37, 0x6F, 0x2F, 0x68, 0x35, 0x3B, 0xA6,
		0xAA, 0x74, 0xE1, 0xA1, 0x3C, 0xE8, 0xB6, 0x99,
		0x95, 0xAB, 0x31, 0x83, 0x1A, 0x01, 0xFB, 0x9A
	};

	//Init library
	//Enable verbose mode is highly recommended
	atsha_set_verbose();
	atsha_set_log_callback(log_callback);

	//Create new server-side emulation handler with obtained data
	struct atsha_handle *handle = atsha_open_server_emulation(8, sn_data, key_data);
	if (handle == NULL) {
		//There is just one reason of error - memory allocation failed
		fprintf(stderr, "Couldn't create server emulation handler.\n");
		return 1;
	}

	//Check operations status
	int status;
	//Place for output
	atsha_big_int response, sn;
	//And some input
	atsha_big_int challenge = {
		.bytes = 32, //32byte input is required
		.data = {
			0x45, 0x78, 0xA1, 0xAB, 0xA4, 0x35, 0xB0, 0x93,
			0x2F, 0x9F, 0x8A, 0xB4, 0x55, 0x9C, 0xA1, 0x1A,
			0x13, 0x6D, 0x3F, 0x03, 0x31, 0x1F, 0x16, 0x92,
			0x6D, 0x2D, 0x46, 0x41, 0x70, 0x2B, 0x79, 0x72
		}
	};

	//Do some funny stuff
	printf("Serial number:\n");
	status = atsha_serial_number(handle, &sn);
	if (status != ATSHA_ERR_OK) {
		printf("Operation failed: %s\n", atsha_error_name(status));
	}
	printf("SN is %u bytes long number:\n", (unsigned)sn.bytes);
	print_number(sn.bytes, sn.data);

	printf("Challenge response:\n");
	status = atsha_challenge_response(handle, challenge, &response);
	if (status != ATSHA_ERR_OK) {
		printf("Operation failed: %s\n", atsha_error_name(status));
	}
	printf("Response is %u bytes long number:\n", (unsigned)response.bytes);
	print_number(response.bytes, response.data);

	//Close all files, free allocated memory etc.
	atsha_close(handle);

	handle = atsha_open();
	printf("Challenge response:\n");
	status = atsha_challenge_response(handle, challenge, &response);
	if (status != ATSHA_ERR_OK) {
		printf("Operation failed: %s\n", atsha_error_name(status));
	}
	printf("Response is %u bytes long number:\n", (unsigned)response.bytes);
	print_number(response.bytes, response.data);

	return 0;
}
