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
#include "operations.h"
#include "tools.h"
#include "api.h"

extern atsha_configuration g_config;

static const size_t POSITION_OPCODE = 1;
static const size_t POSITION_PARAM1 = 2;
static const size_t POSITION_ADDRESS = 3;
static const size_t LINE_BUFFSIZE = 128;

static int emul_hmac(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	(void) raw_packet;
	(void) answer;
	(void) handle;

	return ATSHA_ERR_NOT_IMPLEMENTED;
}

static int emul_read(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	char line[LINE_BUFFSIZE];
	char *line_p = line;

	//Decide if user try to read SN or data
	unsigned char read_from = raw_packet[POSITION_PARAM1] & 0x3;

	if (read_from == IO_MEM_CONFIG) {
		//User want serial number

		//Operation read supports only reading SN from config memory
		if (raw_packet[POSITION_ADDRESS] != 0x00) return ATSHA_ERR_NOT_IMPLEMENTED;

		//Serial number is first line
		rewind(handle->file);
		if (fgets(line, LINE_BUFFSIZE, handle->file) == NULL) return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;

		//Prepare memory for reading operation
		size_t sn_in_memory_len = ATSHA204_SN_BYTE_LEN + 4;
		unsigned char SN[ATSHA204_SLOT_BYTE_LEN]; //this is much more memory, but higher layers want it

		size_t i = 0;
		while (i < sn_in_memory_len) {
			//Make "virtual" hole in answer
			if ((i >= 4) && (i <= 7)) {
				i++;
				continue;
			}
			//skip delimiters
			if (line_p[0] == ' ' || line_p[0] == '\t' || line_p[0] == ';' || line_p[0] == ',' || line_p[0] == ':') {
				line_p++;
				continue;
			}
			//Decode byte representation
			SN[i++] = get_number_from_hex_char(line_p[0], line_p[1]);
			line_p += 2;
		}

		(*answer) = generate_answer_packet(SN, ATSHA204_SLOT_BYTE_LEN); //
		if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	} else if (read_from == IO_MEM_DATA) {
		//User want some slot data

		rewind(handle->file);
		//Adresses starts at multiples of 8; first line is SN
		size_t skip_lines = (raw_packet[POSITION_ADDRESS] / 8) + 1;
		for (size_t i = 0; i < skip_lines; i++) {
			if (fgets(line, LINE_BUFFSIZE, handle->file) == NULL) return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
		}

		//On next line is key that user want
		if (fgets(line, LINE_BUFFSIZE, handle->file) == NULL) return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
		unsigned char key[ATSHA204_SLOT_BYTE_LEN];
		size_t i = 0;
		while (i < ATSHA204_SLOT_BYTE_LEN) {
			if (line_p[0] == ' ' || line_p[0] == '\t' || line_p[0] == ';' || line_p[0] == ',' || line_p[0] == ':') {
				line_p++;
				continue;
			}

			key[i++] = get_number_from_hex_char(line_p[0], line_p[1]);
			line_p += 2;
		}

		(*answer) = generate_answer_packet(key, ATSHA204_SLOT_BYTE_LEN);
		if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	} else {
		return ATSHA_ERR_NOT_IMPLEMENTED;
	}

	return ATSHA_ERR_OK;
}

int emul_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	(*answer) = NULL; //Important init operation!!

	int status;
	switch (raw_packet[POSITION_OPCODE]) {
		case ATSHA204_OPCODE_HMAC:
			status = emul_hmac(handle, raw_packet, answer);
			break;

		case ATSHA204_OPCODE_READ:
			status = emul_read(handle, raw_packet, answer);
			break;

		default:
			status = ATSHA_ERR_NOT_IMPLEMENTED;
			break;
	}

	return status;
}
