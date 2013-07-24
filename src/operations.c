#include <stdlib.h>

#include "operations.h"
#include "atsha204.h"
#include "tools.h"

unsigned char *op_dev_rev() {
	return generate_command_packet(ATSHA204_OPCODE_DEV_REV, 0, 0, NULL, 0);
}

unsigned char *op_dev_rev_recv() {
	return generate_command_packet(ATSHA204_OPCODE_DEV_REV, 0, 0, NULL, 0);
}
