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
