#include <stdlib.h>
#include <stdint.h>
#include <string.h> //memcpy()

#include "operations.h"
#include "atsha204consts.h"
#include "tools.h"

unsigned char *op_dev_rev() {
	return generate_command_packet(ATSHA204_OPCODE_DEV_REV, 0, 0, NULL, 0);
}

uint32_t op_dev_rev_recv(unsigned char *packet) {
	uint32_t res = 0;
	packet++; //skip size

	res |= packet[0] << (3 * 8);
	res |= packet[1] << (2 * 8);
	res |= packet[2] << (1 * 8);
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
	int size = packet[0] - 3; //-3 == -1 count and -2 crc
	*data = (unsigned char *)calloc(size, sizeof(unsigned char));

	if (*data == NULL) return 0;

	memcpy(*data, (packet + 1), size);

	return size;
}

//TODO: find better place for this constants
static const unsigned char READ_FROM_DATA = 2;
static const unsigned char READ_FROM_CONFIG = 0;
static const unsigned char READ_FROM_OTP = 1;
static const unsigned char READ_4_BYTES = 0;
static const unsigned char READ_32_BYTES = 128; //10000000

/**
 * Zone config represents Param1
 */
static unsigned char get_zone_config(unsigned char read_from, unsigned char read_bytes) {
	unsigned char config = 0; //clean config byte
	config = read_from | read_bytes; //only one option of config values is possibe
printf("Config: %02X\n", config);
	return config;
}

//TODO: find better place for this constants
static const unsigned char SLOT_ADDRESSES[] = {
	0x0000, 0x0008, 0x0010, 0x0018,
	0x0020, 0x0028, 0x0030, 0x0038,
	0x0040, 0x0048, 0x0050, 0x0058,
	0x0060, 0x0068, 0x0070, 0x0078
};
/**
 * Slot number is in range 0--15
 */
static unsigned char get_slot_address(unsigned char slot_number) {
	unsigned char address = 0;
	//address |= (SLOT_ADDRESSES[slot_number] << 3);
	address |= SLOT_ADDRESSES[slot_number];
	return address;
}

unsigned char *op_read(unsigned char slot_number) {
	return generate_command_packet(
		ATSHA204_OPCODE_READ,
		get_zone_config(READ_FROM_DATA, READ_32_BYTES),
		(uint16_t)get_slot_address(slot_number),
		NULL,
		0
	);
}

int op_read_recv(unsigned char *packet, unsigned char **data) {
	int size = packet[0] - 3; //-3 == -1 count and -2 crc
	*data = (unsigned char *)calloc(size, sizeof(unsigned char));

	if (*data == NULL) return 0;

	memcpy(*data, (packet + 1), size);

	return size;
}
