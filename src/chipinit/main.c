#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include<string.h>

#include "../libatsha204/atsha204.h"
#include "../libatsha204/atsha204consts.h"
#include "../libatsha204/tools.h"

#define BUFFSIZE_LINE 128
#define BYTESIZE_KEY 32
#define BYTESIZE_OTP 4
#define SLOT_CNT 16

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

static void dump_device(struct atsha_handle *handle) {
	atsha_big_int data;

	printf("Config zone (0x00 - 0x15):\n");
	for (unsigned char addr = 0x00; addr <= 0x15; addr++) {
		sleep(1);
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
		sleep(1);
		atsha_raw_slot_read(handle, slot, &data);
		printf("%2u: ", slot);
		for (size_t i = 0; i < data.bytes; i++) {
			printf("%02X ", data.data[i]);
		}
		printf("\n");
	}
return;
	printf("OTP zone (0x00 - 0x0F):\n");
	for (unsigned char addr = 0x00; addr <= 0x0F; addr++) {
		sleep(1);
		atsha_raw_otp_read(handle, addr, &data);
		printf("0x%02X: ", addr);
		for (size_t i = 0; i < data.bytes; i++) {
			printf("%02X ", data.data[i]);
		}
		printf("\n");
	}
	printf("\n");
}

static bool read_config(FILE *conf, unsigned char *data, size_t line_cnt) {
	char line[BUFFSIZE_LINE];

	for (size_t item = 0; item < SLOT_CNT; item++) {
		if (fgets(line, BUFFSIZE_LINE, conf) == NULL) {
			return false;
		}

		char *line_p = line;
		char *line_end_p = (line_p + strlen(line_p));

		size_t i = 0;
		while (i < line_cnt) {
			if (line_p[0] == ' ' || line_p[0] == '\t' || line_p[0] == ';' || line_p[0] == ',' || line_p[0] == ':') {
				line_p++;
				continue;
			}

			(data+(line_cnt*item))[i++] = get_number_from_hex_char(line_p[0], line_p[1]);
			line_p += 2;

			if (line_p >= line_end_p) {
				return false;
			}
		}
	}

	return true;
}

bool set_otp_mode(struct atsha_handle *handle) {
	atsha_big_int record;
	if (atsha_raw_conf_read(handle, 0x04, &record) != ATSHA_ERR_OK) return false;
	record.data[2] = ATSHA204_CONFIG_OTPMODE_READONLY;
	if (atsha_raw_conf_write(handle, 0x04, record) != ATSHA_ERR_OK) return false;

	return true;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s config\n", argv[0]);
		return 1;
	}

	FILE *conf = fopen(argv[1], "r");
	if (conf == NULL) {
		fprintf(stderr, "Couldn't open config file %s\n", argv[1]);
		return 1;
	}
	//init LIBATSHA204
	atsha_set_verbose();
	atsha_set_log_callback(log_callback);
	//Create LIBATSHA204 handler
	struct atsha_handle *handle = atsha_open_i2c_dev();
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open I2C devidce.\n");
		return 1;
	}
//	set_otp_mode(handle);
//	dump_device(handle);
	//Prepare data structures
	unsigned char data[SLOT_CNT*BYTESIZE_KEY];
	unsigned char otp[SLOT_CNT*BYTESIZE_OTP];
	atsha_big_int number;
	//Read keys
	if (!read_config(conf, data, BYTESIZE_KEY)) {
		fprintf(stderr, "Couldn't read config data (keys).\n");
		return 1;
	}
	//Read OTP items
	if (!read_config(conf, otp, BYTESIZE_OTP)) {
		fprintf(stderr, "Couldn't read config data (OTP).\n");
		return 1;
	}
	//Calculate CRC of zones
	unsigned char crc_data[2], crc_otp[2];
	calculate_crc((unsigned char)(SLOT_CNT*BYTESIZE_KEY), data, crc_data);
	calculate_crc((unsigned char)(SLOT_CNT*BYTESIZE_OTP), otp, crc_otp);
	//Write keys into chip
	/*number.bytes = BYTESIZE_KEY;
	for (size_t item = 0; item < SLOT_CNT; item++) {
		memcpy(number.data, (data + (item * BYTESIZE_KEY)), number.bytes);
		atsha_raw_slot_write(handle, item, number);
		sleep(1);
	}*/
	//Write OTP items into chip
	size_t item = 0;
	number.bytes = BYTESIZE_OTP;
	for (unsigned char addr = 0x00; addr <= 0x0F; addr++) {
		memcpy(number.data, (otp + (item * BYTESIZE_OTP)), number.bytes);
		item++;
		atsha_raw_otp_write(handle, addr, number);
		sleep(1);
	}

//	dump_device(handle);

	fclose(conf);
	atsha_close(handle);

	return 0;
}
