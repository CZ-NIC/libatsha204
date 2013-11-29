/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h> //memcpy()

#include "configuration.h"
#include "operations.h"
#include "atsha204consts.h"
#include "atsha204.h"
#include "tools.h"
#include "api.h"

/*
 * Constants enabling addressing
 */
static const unsigned char SLOT_ADDRESSES[] = {
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
	0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
};

//internal function
static int read_long_data(unsigned char *packet, unsigned char *data) {
	int size = packet[0] - 3; //-3 == -1 count and -2 crc
	if (size > ATSHA_MAX_DATA_SIZE) {
		log_message("operations: read_long_data: size is bigger than max size");
		return 0;
	}

	memcpy(data, (packet + 1), size);

	return size;
}
//internal function
static int just_check_status(unsigned char *packet) {
	if (packet[1] == ATSHA204_STATUS_SUCCES) {
		return ATSHA_ERR_OK;
	}

	log_message("operations: just_check_status: status is not ATSHA204_STATUS_SUCCES");
	return ATSHA_ERR_BAD_COMMUNICATION_STATUS;
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

int op_random_recv(unsigned char *packet, unsigned char *data) {
	return read_long_data(packet, data);
}

unsigned char get_zone_config(unsigned char io_mem, unsigned char io_enc, unsigned char io_cnt) {
	unsigned char config = 0; //clean config byte
	config = io_mem | io_enc | io_cnt; //only one option of config values is possibe
	return config;
}

unsigned char get_lock_config(unsigned char lock_what) {
	unsigned char config = 0; //0 = we need to check CRC
	config |= lock_what;
	return config;
}

unsigned char get_slot_address(unsigned char slot_number) {
	unsigned char address = 0;
	address |= SLOT_ADDRESSES[slot_number];
	return address;
}

unsigned char *op_raw_read(unsigned char zone_config, unsigned char address) {
	return generate_command_packet(ATSHA204_OPCODE_READ, zone_config, (uint16_t)address, NULL, 0);
}

int op_raw_read_recv(unsigned char *packet, unsigned char *data) {
	return read_long_data(packet, data);
}

unsigned char *op_raw_write(unsigned char zone_config, unsigned char address, size_t cnt, unsigned char *data) {
	return generate_command_packet(ATSHA204_OPCODE_WRITE, zone_config, (uint16_t)address, data, cnt);
}

int op_raw_write_recv(unsigned char *packet) {
	return just_check_status(packet);
}

unsigned char *op_nonce(size_t cnt, unsigned char *data) {
	unsigned char USE_MODE = 0x03; //pass-trough mode
	return generate_command_packet(ATSHA204_OPCODE_NONCE, USE_MODE, 0, data, cnt);
}

int op_nonce_recv(unsigned char *packet) {
	return just_check_status(packet);
}

unsigned char *op_hmac(unsigned char address, bool use_sn_in_digest) {
	unsigned char USE_MODE = 0x04; //0x04 3rd bit must match TempKey.SourceFlag
	if (use_sn_in_digest) {
		if (USE_OUR_SN) {
			USE_MODE |= 0x20; //5th bit is 1 == use OTP 0-7
		} else {
			USE_MODE |= 0x40; //6th bit is 1 == use SN
		}
	}

	return generate_command_packet(ATSHA204_OPCODE_HMAC, USE_MODE, (uint16_t)address, NULL, 0);
}

int op_hmac_recv(unsigned char *packet, unsigned char *data) {
	return read_long_data(packet, data);
}

unsigned char *op_mac(unsigned char address, size_t cnt, unsigned char *data, bool use_sn_in_digest) {
	unsigned char USE_MODE = 0x00; //use key slot; read message from input
	if (use_sn_in_digest) {
		if (USE_OUR_SN) {
			USE_MODE |= 0x20; //5th bit is 1 == use OTP 0-7
		} else {
			USE_MODE |= 0x40; //6th bit is 1 == use SN
		}
	}

	return generate_command_packet(ATSHA204_OPCODE_MAC, USE_MODE, (uint16_t)address, data, cnt);
}

int op_mac_recv(unsigned char *packet, unsigned char *data) {
	return read_long_data(packet, data);
}

unsigned char *op_serial_number() {
	return generate_command_packet(ATSHA204_OPCODE_READ, get_zone_config(IO_MEM_CONFIG, IO_RW_NON_ENC, IO_RW_32_BYTES), 0x0000, NULL, 0);
}

int op_serial_number_recv(unsigned char *packet, unsigned char *data) {
	int size = 9;

	data[0] = (packet+1)[0];
	data[1] = (packet+1)[1];
	data[2] = (packet+1)[2];
	data[3] = (packet+1)[3];
	data[4] = (packet+1)[8];
	data[5] = (packet+1)[9];
	data[6] = (packet+1)[10];
	data[7] = (packet+1)[11];
	data[8] = (packet+1)[12];

	return size;
}

unsigned char *op_lock(unsigned char lock_config, const unsigned char *crc) {
	uint16_t crc_int = 0; //clear
	crc_int |= crc[1]; crc_int <<= (1 * 8);
	crc_int |= crc[0];

	return generate_command_packet(ATSHA204_OPCODE_LOCK, lock_config, crc_int, NULL, 0);
}

int op_lock_recv(unsigned char *packet) {
	return just_check_status(packet);
}
