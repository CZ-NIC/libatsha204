#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "../libatsha204/atsha204.h"

static const char *CMD_SN = "serial-number";
static const char *CMD_HMAC = "challenge-response";
static const char *CMD_HWREV = "hw-rev";
static const char *CMD_FILEHMAC = "file-challenge-response";

#define BUFFSIZE 512

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

void print_number(size_t len, unsigned char *data) {
	for (size_t i = 0; i < len; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

void help(char *prgname) {
	fprintf(stderr,
		"Usage: %s [command]\n"
		"Available [command] options:\n"
			"\t%s\t\tprint serial number to stdout\n"
			"\t%s\t\t\tprint hw revision number to stdout\n"
			"\t%s\tprint HMAC response to stdout\n"
			"\t%s\tprint HMAC response to stdout with challenge from file\n"
		"Input/Output on stdin/stdout is in format:\n"
			"\t00112233...\tor\n"
			"\t00 11 22 33...\tor\n"
			"\t00:11:22:33...\tor\n"
			"\t00;11;22;33...\tor\n"
			"\t00,11,22,33...\t\n"
		"\n"
		, prgname, CMD_SN, CMD_HWREV, CMD_HMAC, CMD_FILEHMAC
	);
}

bool read_challenge(char *buff) {
	if (fgets(buff, BUFFSIZE, stdin) == NULL) {
		return false;
	}

	return true;
}

/**
 * This function gets two chars, that are representing hex string, and
 * return real byte value.
 */
unsigned char get_number_from_hex_char(char high, char low) {
	char str[3];
	str[0] = high; str[1] = low; str[2] = '\0';

	return (unsigned char)strtol(str, NULL, 16);
}

bool get_challenge_from_input(char *buff, atsha_big_int *challenge) {
	challenge->bytes = ATSHA_MAX_DATA_SIZE;
	char *end = (buff + strlen(buff));

	size_t i = 0;
	while (i < challenge->bytes) {
		if (buff[0] == ' ' || buff[0] == '\t' || buff[0] == ';' || buff[0] == ',' || buff[0] == ':') {
			buff++;
			continue;
		}

		challenge->data[i++] = get_number_from_hex_char(buff[0], buff[1]);
		buff += 2;

		if (buff >= end) return false;
	}

	return true;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		help(argv[0]);
		return 1;
	}

	atsha_set_verbose();
	atsha_set_log_callback(log_callback);

	int status;
	struct atsha_handle *handle;
	handle = atsha_open();
	if (handle == NULL) {
		fprintf(stderr, "Device couldn't be opened.\n");
		return 3;
	}

	if (strcmp(argv[1], CMD_SN) == 0) {
		atsha_big_int sn;
		status = atsha_serial_number(handle, &sn);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, "Serial number error: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}

		print_number(sn.bytes, sn.data);
	} else if (strcmp(argv[1], CMD_HWREV) == 0) {
		atsha_big_int sn;
		status = atsha_serial_number(handle, &sn);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, "HW revision number error: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}
		/*
		Serial number has 8bytes
		4bytes of HW revision number and 4bytes unique number.
		So... use only first 4 bytes.
		*/
		print_number(4, sn.data);

	} else if (strcmp(argv[1], CMD_HMAC) == 0) {
		char buff[BUFFSIZE];
		atsha_big_int challenge;
		atsha_big_int response;

		if (!read_challenge(buff)) {
			fprintf(stderr, "Input couldn't be read.\n");
			atsha_close(handle);
			return 2;
		}

		if (!get_challenge_from_input(buff, &challenge)) {
			fprintf(stderr, "Input couldn't be converted.\n");
			atsha_close(handle);
			return 2;
		}

		status = atsha_challenge_response(handle, challenge, &response);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, "Challenge response error: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}

		print_number(response.bytes, response.data);


	} else {
		help(argv[0]);
		atsha_close(handle);
		return 1;
	}

	atsha_close(handle);

	return 0;
}
