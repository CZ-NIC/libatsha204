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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "../libatsha204/atsha204.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>

void testing_log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

int hmac_impl_test(atsha_big_int *nonce, atsha_big_int key) {
	unsigned char output[32];
	size_t message_len = 32+32+1+1+2+8+3+1+4+2+2; //88
	unsigned char message[message_len];

	size_t i;
	for (i = 0; i < 32; i++) {
		message[i] = 0;
	}

	for (i = 32; i < 64; i++) {
		message[i] = nonce->data[i-32];
	}
	message[64] = 0x11;
	message[65] = 0x04;
	message[66] = 0x08;
	message[67] = 0x00;

	for (i = 68; i < 76; i++) {
		message[i] = 0;
	}

	//////////////////
	message[76] = 0x00;
	message[77] = 0x00;
	message[78] = 0x00;
	//////////////////
	message[79] = 0xEE;
	//////////////////
	message[80] = 0x00;
	message[81] = 0x00;
	message[82] = 0x00;
	message[83] = 0x00;
	//////////////////
	message[84] = 0x01;
	message[85] = 0x23;
	//////////////////
	message[86] = 0x00;
	message[87] = 0x00;
	//////////////////

	for (i = 0; i < 32; i++) {
		if (!(i%8)) fprintf(stderr, "\n");
		fprintf(stderr, "0x%02X ", nonce->data[i]);
	}
	fprintf(stderr, "\n");

	for (i = 0; i < 88; i++) {
		if (!(i%8)) fprintf(stderr, "\n");
		fprintf(stderr, "0x%02X ", message[i]);
	}
	fprintf(stderr, "\n");

	unsigned int ret_len;
	unsigned char *ret_status;
	ret_status = HMAC(EVP_sha256(), key.data, key.bytes, message, message_len, output, &ret_len);
	if (ret_status == NULL || ret_len != 32) return ATSHA_ERR_BAD_COMMUNICATION_STATUS;

	fprintf(stderr, "SW HMAC: \t"); for (size_t i = 0; i < 32; i++) { printf("%02X ", output[i]); } printf("\n");

	return ATSHA_ERR_OK;
}

int hmac(struct atsha_handle *handle) {
	int status;
	atsha_big_int number;
	atsha_big_int digest;
	atsha_big_int key;
	fprintf(stderr, "\n\n============================================================================\n\n");
	status = atsha_random(handle, &number);
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Nonce: \t"); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	status = atsha_slot_read(handle, &key);
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Key: \t"); for (size_t i = 0; i < key.bytes; i++) { printf("%02X ", key.data[i]); } printf("\n");
	}

	hmac_impl_test(&number, key);

	status = atsha_low_challenge_response(handle, atsha_find_slot_number(handle), number, &digest, false);
	fprintf(stderr, "HMAC digest status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "HW HMAC: \t"); for (size_t i = 0; i < digest.bytes; i++) { printf("%02X ", digest.data[i]); } printf("\n");
	}

	fprintf(stderr, "\n\n============================================================================\n\n");

	return status;
}

int mac_impl_test(atsha_big_int *nonce, atsha_big_int key) {
	unsigned char output[32];
	size_t message_len = 32+32+1+1+2+8+3+1+4+2+2; //88
	unsigned char message[message_len];

	size_t i;
	for (i = 0; i < 32; i++) {
		message[i] = key.data[i];
	}

	for (i = 32; i < 64; i++) {
		message[i] = nonce->data[i-32];
	}
	message[64] = 0x08;
	message[65] = 0x00;
	message[66] = 0x08;
	message[67] = 0x00;

	for (i = 68; i < 76; i++) {
		message[i] = 0;
	}

	//////////////////
	message[76] = 0x00;
	message[77] = 0x00;
	message[78] = 0x00;
	//////////////////
	message[79] = 0xEE;
	//////////////////
	message[80] = 0x00;
	message[81] = 0x00;
	message[82] = 0x00;
	message[83] = 0x00;
	//////////////////
	message[84] = 0x01;
	message[85] = 0x23;
	//////////////////
	message[86] = 0x00;
	message[87] = 0x00;
	//////////////////

	for (i = 0; i < 32; i++) {
		if (!(i%8)) fprintf(stderr, "\n");
		fprintf(stderr, "0x%02X ", nonce->data[i]);
	}
	fprintf(stderr, "\n");

	for (i = 0; i < 88; i++) {
		if (!(i%8)) fprintf(stderr, "\n");
		fprintf(stderr, "0x%02X ", message[i]);
	}
	fprintf(stderr, "\n");

	unsigned char *ret_status;
	ret_status = SHA256(message, message_len, output);
	if (ret_status == NULL) return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
	fprintf(stderr, "SW MAC: \t"); for (size_t i = 0; i < 32; i++) { printf("%02X ", output[i]); } printf("\n");

	return ATSHA_ERR_OK;
}

int mac(struct atsha_handle *handle) {
	int status;
	atsha_big_int number;
	atsha_big_int digest;
	atsha_big_int key;
	fprintf(stderr, "\n\n============================================================================\n\n");
	status = atsha_random(handle, &number);
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Nonce: \t"); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	status = atsha_slot_read(handle, &key);
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "Key: \t"); for (size_t i = 0; i < key.bytes; i++) { printf("%02X ", key.data[i]); } printf("\n");
	}

	mac_impl_test(&number, key);

	status = atsha_low_challenge_response_mac(handle, atsha_find_slot_number(handle), number, &digest, false);
	fprintf(stderr, "MAC digest status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "HW MAC: \t"); for (size_t i = 0; i < digest.bytes; i++) { printf("%02X ", digest.data[i]); } printf("\n");
	}

	fprintf(stderr, "\n\n============================================================================\n\n");

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

	hmac(handle);
	mac(handle);

	atsha_close(handle);

	return 0;
}
