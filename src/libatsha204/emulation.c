#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>

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

static int emul_nonce(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	memcpy(handle->nonce, (raw_packet + 5), ATSHA204_SLOT_BYTE_LEN);

	unsigned char data[1];
	data[0] = ATSHA204_STATUS_SUCCES;
	*answer = generate_answer_packet(data, 1);
	if (*answer == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	return ATSHA_ERR_OK;
}

static int emul_hmac(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	unsigned char output[32];
	size_t message_len = 32+32+1+1+2+8+3+1+4+2+2; //88
	unsigned char message[message_len];

	bool use_sn;
	if ((raw_packet[POSITION_PARAM1] & 0x40) == 0) {
		use_sn = false;
	} else {
		use_sn = true;
	}
	//Start of message
	//////////////////
	for (size_t i = 0; i < 32; i++) {
		message[i] = 0;
	}
	//////////////////
	for (size_t i = 32; i < 64; i++) {
		message[i] = handle->nonce[i-32];
	}
	//////////////////
	message[64] = ATSHA204_OPCODE_HMAC; //opcode
	message[65] = raw_packet[POSITION_PARAM1]; //mode
	message[66] = raw_packet[POSITION_PARAM1 + 1]; //param2 alias slotID
	message[67] = raw_packet[POSITION_PARAM1 + 2]; //param2 alias slotID
	//////////////////
	message[68] = 0x00; //8bytes OTP[0:7] - we will never use it!!
	message[69] = 0x00;
	message[70] = 0x00;
	message[71] = 0x00;
	message[72] = 0x00;
	message[73] = 0x00;
	message[74] = 0x00;
	message[75] = 0x00;
	//////////////////
	message[76] = 0x00; //8bytes OTP[8:10] - we will never use it!!
	message[77] = 0x00;
	message[78] = 0x00;
	//////////////////
	message[79] = handle->sn[8]; //0xEE constant
	//////////////////
	if (use_sn) {
		message[80] = handle->sn[4];
		message[81] = handle->sn[5];
		message[82] = handle->sn[6];
		message[83] = handle->sn[7];
	} else {
		message[80] = 0x00;
		message[81] = 0x00;
		message[82] = 0x00;
		message[83] = 0x00;
	}
	//////////////////
	message[84] = handle->sn[0]; //0x01 constant
	message[85] = handle->sn[1]; //0x02 constant
	//////////////////
	if (use_sn) {
		message[86] = handle->sn[2];
		message[87] = handle->sn[3];
	} else {
		message[86] = 0x00;
		message[87] = 0x00;
	}
	//////////////////
	//End of message

	atsha_big_int key;
	if (atsha_low_slot_read(handle, raw_packet[POSITION_PARAM1+1], &key) != ATSHA_ERR_OK) {
		log_message("emulation: emul_hmac: Bad status code: atsha_low_slot_read");
		return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
	}

	unsigned int ret_len;
	unsigned char *ret_status;
	ret_status = HMAC(EVP_sha256(), key.data, key.bytes, message, message_len, output, &ret_len);

	if (ret_status == NULL || ret_len != 32) {
		log_message("emulation: emul_hmac: Bad status code: HMAC (libopenssl)");
		return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
	}

	(*answer) = generate_answer_packet(output, 32);
	if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	return ATSHA_ERR_OK;
}

static int emul_mac(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	unsigned char output[32];
	size_t message_len = 32+32+1+1+2+8+3+1+4+2+2; //88
	unsigned char message[message_len];

	atsha_big_int key;
	if (atsha_low_slot_read(handle, raw_packet[POSITION_PARAM1+1], &key) != ATSHA_ERR_OK) {
		log_message("emulation: emul_mac: Bad status code: atsha_low_slot_read");
		return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
	}

	bool use_sn;
	if ((raw_packet[POSITION_PARAM1] & 0x40) == 0) {
		use_sn = false;
	} else {
		use_sn = true;
	}
	//Start of message
	//////////////////
	for (size_t i = 0; i < 32; i++) {
		message[i] = key.data[i];
	}
	//////////////////
	for (size_t i = 32; i < 64; i++) {
		message[i] = raw_packet[i-32+5]; //+1 == skip count parameter
	}
	//////////////////
	message[64] = ATSHA204_OPCODE_MAC; //opcode
	message[65] = raw_packet[POSITION_PARAM1]; //mode
	message[66] = raw_packet[POSITION_PARAM1 + 1]; //param2 alias slotID
	message[67] = raw_packet[POSITION_PARAM1 + 2]; //param2 alias slotID
	//////////////////
	message[68] = 0x00; //8bytes OTP[0:7] - we will never use it!!
	message[69] = 0x00;
	message[70] = 0x00;
	message[71] = 0x00;
	message[72] = 0x00;
	message[73] = 0x00;
	message[74] = 0x00;
	message[75] = 0x00;
	//////////////////
	message[76] = 0x00; //8bytes OTP[8:10] - we will never use it!!
	message[77] = 0x00;
	message[78] = 0x00;
	//////////////////
	message[79] = handle->sn[8]; //0xEE constant
	//////////////////
	if (use_sn) {
		message[80] = handle->sn[4];
		message[81] = handle->sn[5];
		message[82] = handle->sn[6];
		message[83] = handle->sn[7];
	} else {
		message[80] = 0x00;
		message[81] = 0x00;
		message[82] = 0x00;
		message[83] = 0x00;
	}
	//////////////////
	message[84] = handle->sn[0]; //0x01 constant
	message[85] = handle->sn[1]; //0x02 constant
	//////////////////
	if (use_sn) {
		message[86] = handle->sn[2];
		message[87] = handle->sn[3];
	} else {
		message[86] = 0x00;
		message[87] = 0x00;
	}
	//////////////////
	//End of message

	unsigned char *ret_status;
	ret_status = SHA256(message, message_len, output);
	if (ret_status == NULL) {
		log_message("emulation: emul_mac: Bad status code: SHA256 (libopenssl)");
		return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
	}

	(*answer) = generate_answer_packet(output, 32);
	if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	return ATSHA_ERR_OK;
}

static int emul_random(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	(void) raw_packet;
	(void) handle;

	unsigned char data[] = {
		0x55, 0xE3, 0xD9, 0x65, 0x6A, 0x82, 0x69, 0x33,
		0xB2, 0xAF, 0x9B, 0xCA, 0x64, 0x5C, 0x49, 0x0D,
		0x09, 0x27, 0xC3, 0x82, 0x0C, 0x32, 0x58, 0xBC,
		0x8D, 0x23, 0xEE, 0xC4, 0x04, 0xF3, 0xC4, 0x14
	};

	(*answer) = generate_answer_packet(data, ATSHA204_SLOT_BYTE_LEN);
	if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	return ATSHA_ERR_OK;
}

static int emul_read(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	char line[BUFFSIZE_LINE];
	char *line_p = line;

	//Decide if user try to read SN or data
	unsigned char read_from = raw_packet[POSITION_PARAM1] & 0x3;

	if (read_from == IO_MEM_CONFIG) {
		//User want serial number

		//Operation read supports only reading SN from config memory
		if (raw_packet[POSITION_ADDRESS] != 0x00) {
			log_message("emulation: emul_read: Reading only 0x00 address from config zone is allowed");
			return ATSHA_ERR_NOT_IMPLEMENTED;
		}

		//Prepare memory for reading operation
		size_t sn_in_memory_len = ATSHA204_SN_BYTE_LEN + 4;
		unsigned char SN[ATSHA204_SLOT_BYTE_LEN]; //this is much more memory, but higher layers want it
		clear_buffer(SN, ATSHA204_SLOT_BYTE_LEN);

		if (handle->is_srv_emulation) {
			SN[0] = handle->sn[0]; SN[1] = handle->sn[1]; SN[2] = handle->sn[2];
			SN[3] = handle->sn[3]; SN[8] = handle->sn[4]; SN[9] = handle->sn[5];
			SN[10] = handle->sn[6]; SN[11] = handle->sn[7]; SN[12] = handle->sn[8];
		} else {
			//Serial number is first line
			rewind(handle->file);
			if (fgets(line, BUFFSIZE_LINE, handle->file) == NULL) {
				log_message("emulation: emul_read: read SN (bad file format)");
				return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
			}

			char *line_end_p = (line_p + strlen(line_p));

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

				if (line_p >= line_end_p) {
					log_message("emulation: emul_read: read SN (input too short)");
					return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
				}
			}
		}

		(*answer) = generate_answer_packet(SN, ATSHA204_SLOT_BYTE_LEN);
		if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	} else if (read_from == IO_MEM_DATA) {
		//User want some slot data

		if (handle->is_srv_emulation) {
			(*answer) = generate_answer_packet(handle->key, ATSHA204_SLOT_BYTE_LEN);
			if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
		} else {
			rewind(handle->file);
			//Adresses starts at multiples of 8; first line is SN
			size_t skip_lines = (raw_packet[POSITION_ADDRESS] / 8) + 1;
			for (size_t i = 0; i < skip_lines; i++) {
				if (fgets(line, BUFFSIZE_LINE, handle->file) == NULL) {
					log_message("emulation: emul_read: skip keys (bad file format)");
					return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
				}
			}

			//On next line is key that user want
			if (fgets(line, BUFFSIZE_LINE, handle->file) == NULL) {
				log_message("emulation: emul_read: read requested key (bad file format)");
				return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
			}

			char *line_end_p = (line_p + strlen(line_p));

			unsigned char key[ATSHA204_SLOT_BYTE_LEN];
			size_t i = 0;
			while (i < ATSHA204_SLOT_BYTE_LEN) {
				if (line_p[0] == ' ' || line_p[0] == '\t' || line_p[0] == ';' || line_p[0] == ',' || line_p[0] == ':') {
					line_p++;
					continue;
				}

				key[i++] = get_number_from_hex_char(line_p[0], line_p[1]);
				line_p += 2;

				if (line_p >= line_end_p) {
					log_message("emulation: emul_read: read key (input too short)");
					return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
				 }
			}

			(*answer) = generate_answer_packet(key, ATSHA204_SLOT_BYTE_LEN);
			if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
		}

	} else if (read_from == IO_MEM_OTP) {
		if (handle->is_srv_emulation) {
			log_message("emulation: emul_read: OTP zone not supporten in server emulation mode.");
			return ATSHA_ERR_NOT_IMPLEMENTED;
		} else {
			rewind(handle->file);

			//Skip SN and All key slotss
			for (size_t i = 0; i < 17; i++) {
				if (fgets(line, BUFFSIZE_LINE, handle->file) == NULL) {
					log_message("emulation: emul_read: skip keys (bad file format)");
					return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
				}
			}

			//Skip SN and All key slotss
			for (size_t i = 0; i < raw_packet[POSITION_ADDRESS]; i++) {
				if (fgets(line, BUFFSIZE_LINE, handle->file) == NULL) {
					log_message("emulation: emul_read: skip OTP records (bad file format)");
					return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
				}
			}

			//On next line is key that user want
			if (fgets(line, BUFFSIZE_LINE, handle->file) == NULL) {
				log_message("emulation: emul_read: read requested OTP record (bad file format)");
				return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
			}

			char *line_end_p = (line_p + strlen(line_p));

			unsigned char data[ATSHA204_OTP_BYTE_LEN];
			size_t i = 0;
			while (i < ATSHA204_OTP_BYTE_LEN) {
				if (line_p[0] == ' ' || line_p[0] == '\t' || line_p[0] == ';' || line_p[0] == ',' || line_p[0] == ':') {
					line_p++;
					continue;
				}

				data[i++] = get_number_from_hex_char(line_p[0], line_p[1]);
				line_p += 2;

				if (line_p >= line_end_p) {
					log_message("emulation: emul_read: read requested OTP record (input too short)");
					return ATSHA_ERR_CONFIG_FILE_BAD_FORMAT;
				}
			}

			(*answer) = generate_answer_packet(data, ATSHA204_OTP_BYTE_LEN);
			if ((*answer) == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
		}
	} else {
		log_message("emulation: emul_read: Unknown memory type to read.");
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

		case ATSHA204_OPCODE_MAC:
			status = emul_mac(handle, raw_packet, answer);
			break;

		case ATSHA204_OPCODE_READ:
			status = emul_read(handle, raw_packet, answer);
			break;

		case ATSHA204_OPCODE_NONCE:
			status = emul_nonce(handle, raw_packet, answer);
			break;

		case ATSHA204_OPCODE_RANDOM:
			status = emul_random(handle, raw_packet, answer);
			break;

		default:
			log_message("emulation: requested opconde not implemented");
			status = ATSHA_ERR_NOT_IMPLEMENTED;
			break;
	}

	return status;
}
