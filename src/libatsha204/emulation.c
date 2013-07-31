#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>

#include "atsha204.h"
#include "atsha204consts.h"
#include "configuration.h"
#include "communication.h"
#include "tools.h"
#include "api.h"

extern atsha_configuration g_config;

static const size_t OPCODE_POSITION = 1;

static int emul_hmac(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	(void) raw_packet;
	(void) answer;
	(void) handle;

	return ATSHA_ERR_NOT_IMPLEMENTED;
}

int emul_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	int status;

	if (raw_packet[OPCODE_POSITION] == ATSHA204_OPCODE_HMAC) {
		status = emul_hmac(handle, raw_packet, answer);
	} else {
		status = ATSHA_ERR_NOT_IMPLEMENTED;
	}

	return status;
}
