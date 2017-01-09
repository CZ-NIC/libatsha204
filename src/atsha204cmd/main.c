#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <openssl/sha.h>
#include <assert.h>
#include <linux/random.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "../libatsha204/atsha204.h"
#include "../libatsha204/tools.h"
#include "../libatsha204/atsha204consts.h"

static const char *CMD_SN = "serial-number";
static const char *CMD_HMAC = "challenge-response";
static const char *CMD_HWREV = "hw-rev";
static const char *CMD_FILEHMAC = "file-challenge-response";
static const char *CMD_MAC = "mac";
static const char *CMD_RND = "random";
static const char *CMD_FEED = "feed-entropy";

#define RANDOM_DEVICE       "/dev/random"

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
			"\t%s n\tprint n MAC address to stdout\n"
			"\t%s\tprint 32 raw random bytes to stdout\n"
			"\t%s\tfeed 32 raw random bytes to the kernel entropy source\n"
		"Input/Output on stdin/stdout (except MAC addresses) is in format:\n"
			"\t00112233...\tor\n"
			"\t00 11 22 33...\tor\n"
			"\t00:11:22:33...\tor\n"
			"\t00;11;22;33...\tor\n"
			"\t00,11,22,33...\t\n"
		"\n"
		, prgname, CMD_SN, CMD_HWREV, CMD_HMAC, CMD_FILEHMAC, CMD_MAC, CMD_RND, CMD_FEED
	);
}

bool read_challenge(char *buff) {
	if (fgets(buff, BUFFSIZE, stdin) == NULL) {
		return false;
	}

	return true;
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

bool get_file_sha256(atsha_big_int *abi, FILE *stream) {
	if (stream == NULL) {
		return false;
	}
	abi->bytes = 32;

	unsigned char buff[BUFFSIZE];
	size_t cnt = 0;

	//Init SHA256
	SHA256_CTX sha_context;
	SHA256_Init(&sha_context);

	do {
		cnt = fread(buff, 1, BUFFSIZE, stream);
		SHA256_Update(&sha_context, buff, cnt);
	} while (cnt != 0);

	SHA256_Final(abi->data, &sha_context);

	return true;
}

int main(int argc, char **argv) {
	if (argc < 2) {
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

	} else if (strcmp(argv[1], CMD_RND) == 0) {
		atsha_big_int sn;
		status = atsha_random(handle, &sn);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, "Random numer generation error: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}
		for(int i = 0; i < sn.bytes; i++) {
		    putchar(sn.data[i]);
		}

	} else if (strcmp(argv[1], CMD_FEED) == 0) {
		atsha_big_int sn;
		status = atsha_random(handle, &sn);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, "Random numer generation error: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}
		/* Issue the ioctl to increase the entropy count */
		int random_fd = open(RANDOM_DEVICE, O_RDWR);
		if (random_fd < 0) {
			fprintf(stderr, "Can't open random device %s\n", strerror(errno));
			atsha_close(handle);
			return 3;
		}
		struct rand_pool_info *rand = malloc(sizeof(struct rand_pool_info) + sn.bytes);
		if (!rand) {
			fprintf(stderr, "Can't allocate memory\n");
			atsha_close(handle);
			return 3;
		}
		rand->buf_size = sn.bytes;
		rand->entropy_count = sn.bytes * 8;
		memcpy(rand->buf, sn.data, sn.bytes);
		if (ioctl(random_fd, RNDADDENTROPY, rand) < 0) {
			fprintf(stderr, "RNDADDENTROPY ioctl() failed: %s\n", strerror(errno));
			atsha_close(handle);
			close(random_fd);
			return 3;
		}
		close(random_fd);
		free(rand);

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

	} else if (strcmp(argv[1], CMD_FILEHMAC) == 0) {
		atsha_big_int challenge;
		atsha_big_int response;

		if (!get_file_sha256(&challenge, stdin)) {
			fprintf(stderr, "Input couldn't be read.\n");
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


	} else if ((strcmp(argv[1], CMD_MAC) == 0) && (argc == 3)) {
		unsigned char n = atoi(argv[2]);
		if (n <= 0) {
			fprintf(stderr, "Bad MAC address count requested.\n");
			return 1;
		}

		atsha_big_int prefix;
		atsha_big_int addr;

		status = atsha_raw_otp_read(handle, ATSHA204_OTP_MEMORY_MAP_MAC_PREFIX, &prefix);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, ": Get MAC address prefix failed: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}

		status = atsha_raw_otp_read(handle, ATSHA204_OTP_MEMORY_MAP_MAC_ADDR, &addr);
		if (status != ATSHA_ERR_OK) {
			fprintf(stderr, "Get MAC address suffix failed: %s\n", atsha_error_name(status));
			atsha_close(handle);
			return 3;
		}

		assert(sizeof(unsigned int) >= 4);

		unsigned int mac_as_number = 0, mac_as_number_orig = 0;
		unsigned char tmp_mac[6];

		memcpy(tmp_mac, (prefix.data+1), 3);

		mac_as_number_orig |= (addr.data[1] << 8*2);
		mac_as_number_orig |= (addr.data[2] << 8*1);
		mac_as_number_orig |= addr.data[3];

		if (mac_as_number_orig > (((unsigned int)0xFFFFFF)-n)) {
			fprintf(stderr, "MAC address count is to big!\n");
			return 4;
		}

		for (char i = 0; i < n; i++) {
			mac_as_number = mac_as_number_orig;
			mac_as_number_orig++;

			tmp_mac[5] = mac_as_number & 0xFF; mac_as_number >>= 8;
			tmp_mac[4] = mac_as_number & 0xFF; mac_as_number >>= 8;
			tmp_mac[3] = mac_as_number & 0xFF; mac_as_number >>= 8;

			printf("%02X:%02X:%02X:%02X:%02X:%02X\n", tmp_mac[0], tmp_mac[1], tmp_mac[2], tmp_mac[3], tmp_mac[4], tmp_mac[5]);
		}

	} else {
		help(argv[0]);
		atsha_close(handle);
		return 1;
	}

	atsha_close(handle);

	return 0;
}
