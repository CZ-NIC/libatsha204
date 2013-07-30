#include <stdlib.h>
#include <stdint.h>
#include <string.h> //memcpy()

#include "configuration.h"
#include "operations.h"
#include "atsha204consts.h"
#include "atsha204.h"
#include "tools.h"

/**
 * Constants enabling addressing
 */
static const unsigned char SLOT_ADDRESSES[] = {
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
	0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
};

static const unsigned char SLOT_CONFIG_ADDRESSES[] = {
	0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08,
	0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C
};

static int read_long_data(unsigned char *packet, unsigned char **data) {
	int size = packet[0] - 3; //-3 == -1 count and -2 crc
	*data = (unsigned char *)calloc(size, sizeof(unsigned char));

	if (*data == NULL) return 0;

	memcpy(*data, (packet + 1), size);

	return size;
}

unsigned char *op_dev_rev() {
	return generate_command_packet(ATSHA204_OPCODE_DEV_REV, 0, 0, NULL, 0);
}

uint32_t op_dev_rev_recv(unsigned char *packet) {
	uint32_t res = 0;
	packet++; //skip size

	res |= packet[0]; res <<= (3 * 8);
	res |= packet[1]; res <<= (2 * 8);
	res |= packet[2]; res <<= (1 * 8);
	res |= packet[3];

	return res;
}

unsigned char *op_random() {
	/**
	 * Mode 0x00 has better security
	 * Mode 0x01 recycles seed stored in EEPROM
	 */
	unsigned char USE_MODE = 0x00;
	return generate_command_packet(ATSHA204_OPCODE_RANDOM, USE_MODE, 0, NULL, 0);
}

int op_random_recv(unsigned char *packet, unsigned char **data) {
	return read_long_data(packet, data);
}

unsigned char get_zone_config(unsigned char io_mem, unsigned char io_enc, unsigned char io_cnt) {
	unsigned char config = 0; //clean config byte
	config = io_mem | io_enc | io_cnt; //only one option of config values is possibe
	return config;
}

unsigned char get_slot_address(unsigned char slot_number) {
	unsigned char address = 0;
	address |= SLOT_ADDRESSES[slot_number];
	return address;
}

unsigned char get_slot_config_address(unsigned char slot_number) {
	unsigned char address = 0;
	address |= SLOT_CONFIG_ADDRESSES[slot_number];
	return address;
}

uint16_t decode_slot_config(unsigned char slot_number, unsigned char *data) {
	uint16_t config_word = 0;
	size_t offset = (slot_number%2 == 0) ? 0 : 2;

	config_word |= data[0 + offset];
	config_word <<= 8;
	config_word |= data[1 + offset];

	return config_word;
}

uint32_t decode_serial_number(unsigned char *data) {
	uint32_t res = 0;

	res |= data[0]; res <<= (3 * 8);
	res |= data[1]; res <<= (2 * 8);
	res |= data[2]; res <<= (1 * 8);
	res |= data[3];

	return res;
}

unsigned char *op_raw_read(unsigned char zone_config, unsigned char address) {
	return generate_command_packet(ATSHA204_OPCODE_READ, zone_config, (uint16_t)address, NULL, 0);
}

int op_raw_read_recv(unsigned char *packet, unsigned char **data) {
	return read_long_data(packet, data);
}

unsigned char *op_raw_write(unsigned char zone_config, unsigned char address, size_t cnt, unsigned char *data) {
	return generate_command_packet(ATSHA204_OPCODE_WRITE, zone_config, (uint16_t)address, data, cnt);
}

int op_raw_write_recv(unsigned char *packet) {
	if (packet[1] == ATSHA204_STATUS_SUCCES) {
		return ATSHA_ERR_OK;
	}

	return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
}
