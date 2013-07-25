#include <stdlib.h>
#include <stdint.h>

#include "operations.h"
#include "atsha204.h"
#include "tools.h"

unsigned char *op_dev_rev() {
	return generate_command_packet(ATSHA204_OPCODE_DEV_REV, 0, 0, NULL, 0);
}

uint32_t op_dev_rev_recv(unsigned char *packet) {
	uint32_t res = 0;

	for (size_t i = 0; i < 4; i++) {
		res = packet[1+i];
		res <<= 8;
	}
}
