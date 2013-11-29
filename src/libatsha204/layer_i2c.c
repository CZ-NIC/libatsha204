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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <mpsse.h>

#include "atsha204.h"
#include "atsha204consts.h"
#include "configuration.h"
#include "communication.h"
#include "tools.h"
#include "api.h"

extern atsha_configuration g_config;

void i2c_wait() {
	usleep(ATSHA204_I2C_CMD_TOUT);
}

static int i2c_read(struct atsha_handle *handle, unsigned char **answer) {
	unsigned char wr_addr[] = { ATSHA204_I2C_ADDRESS };
	unsigned char *data;
	int status;

	wr_addr[0] |= 0x01;

	for (int i = 0; i < ATSHA204_I2C_IO_ERR_RESPONSE_TRIES; i++) {
		status = Start(handle->i2c);
		if (status != MPSSE_OK) {
			log_message("layer_i2c: i2c_read: Start");
			return ATSHA_ERR_COMMUNICATION;
		}

		status = Write(handle->i2c, (char *)wr_addr, 1);
		if (status != MPSSE_OK) {
			log_message("layer_i2c: i2c_read: Write addr");
			return ATSHA_ERR_COMMUNICATION;
		}

		data = (unsigned char *)Read(handle->i2c, BUFFSIZE_I2C);
		if (data == NULL) {
			log_message("layer_i2c: i2c_command: No data read");
			return ATSHA_ERR_COMMUNICATION;
		}

		status = Stop(handle->i2c);
		if (status != MPSSE_OK) {
			log_message("layer_i2c: i2c_read: Stop");
			return ATSHA_ERR_COMMUNICATION;
		}

		if (data[0] != ATSHA204_I2C_IO_ERR_RESPONSE) {
			*answer = calloc(data[0], sizeof(char));
			if (*answer == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

			memcpy(*answer, data, data[0]);
			free(data);

			return ATSHA_ERR_OK;
		}

		i2c_wait();
	}

	return ATSHA_ERR_COMMUNICATION;
}

int i2c_wake(struct atsha_handle *handle, unsigned char **answer) {
	unsigned char wr_wake[] = { 0x00 };
	int status;

	status = SetClock(handle->i2c, ATSHA204_I2C_WAKE_CLOCK);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: SetClock - set frequency for wake");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Start(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: Start");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)wr_wake, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: Write");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Stop(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: Stop");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = SetClock(handle->i2c, FOUR_HUNDRED_KHZ);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: SetClock - set working frequency");
		return ATSHA_ERR_COMMUNICATION;
	}

	i2c_wait();

	status = i2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}

int i2c_idle(struct atsha_handle *handle) {
	unsigned char wr_idle[] = { ATSHA204_I2C_WA_IDLE };
	unsigned char wr_addr[] = { ATSHA204_I2C_ADDRESS };
	int status;

	status = Start(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_idle: Start");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)wr_addr, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_idle: Write addr");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)wr_idle, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_idle: Write idle");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Stop(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_idle: Stop");
		return ATSHA_ERR_COMMUNICATION;
	}

	i2c_wait();

	return ATSHA_ERR_OK;
}

int i2c_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	unsigned char wr_addr[] = { ATSHA204_I2C_ADDRESS };
	unsigned char wr_cmd[] = { ATSHA204_I2C_WA_COMMAND };
	int status;

	status = Start(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Start");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)wr_addr, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Write addr");
		return ATSHA_ERR_COMMUNICATION;
	}
//TODO: write NACK check here
	status = Write(handle->i2c, (char *)wr_cmd, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Write addr");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)raw_packet, raw_packet[0]);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Write packet");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Stop(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Stop");
		return ATSHA_ERR_COMMUNICATION;
	}

	i2c_wait();

	status = i2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}

//Code bellow this point is muster of communication
#if 0
int maiXYZ(void) {
	char *data = NULL;
	size_t data_len = 128;
	struct mpsse_context *i2c = NULL;
	int ack;
	int status;
	/*char wrdata[] = {
		0x03,
		0x07,
		0x30,
		0x00,
		0x00,
		0x00,
		0x03,
		0x5d
	 };*/

	 char wrdata[] = {
		0x03,
		0x07,
		0x1B,
		0x00,
		0x00,
		0x00,
		0x24,
		0xcd
	 };

	char wrtransmit[] = {
		0x88
	};

	char wrzero[] = {
		0x00
	};

	 char wraddr[] = {
		ADDR
	};
printf("OK: %d\n", MPSSE_OK);
	i2c = MPSSE(I2C, 10000, MSB); //# Initialize libmpsse for I2C operations at 400kHz
	if (i2c == NULL) return 1;


	status = Start(i2c); printf("Status: %d\n", status);
	SendAcks(i2c);

	status = Write(i2c, wrzero, 1); printf("Status: %d\n", status);
	printf("ACK: %d\n", GetAck(i2c));
	status = Stop(i2c); printf("Status: %d\n", status);
	usleep(10000);

	status = Start(i2c);
	status = Write(i2c, wraddr, 1); printf("Status: %d\n", status);
	printf("ACK: %d\n", GetAck(i2c));

	status = Write(i2c, wrdata, 8); printf("Status: %d\n", status);
	printf("ACK: %d\n", GetAck(i2c));
	status = Stop(i2c); printf("Status: %d\n", status);
	usleep(10000);

	status = Start(i2c);
	wraddr[0] |= 1;
	status = Write(i2c, wraddr, 1); printf("Status: %d\n", status);
	printf("ACK: %d\n", GetAck(i2c));
	data = Read(i2c, 10); printf("Status: %d\n", status);

	//status = Write(i2c, wrtransmit, 1); printf("Status: %d\n", status);
	//printf("ACK: %d\n", GetAck(i2c));

//	data = Read(i2c, 1); printf("Status: %d\n", status);
//	Read(i2c, 1); //Read one last "dummy" byte from the I2C slave in order to send the NACK


	Stop(i2c); //Send I2C stop condition
	Close(i2c); //Deinitialize libmpsse

	if (data == NULL) {
		printf("No data\n");
	} else {
		size_t i;
		for (i = 0; i < 40; i++) {
			if ((i % 8) == 0) printf("\n");
			printf("0x%02X ", data[i]);
		}
		printf("\n");
	}

	free(data);
	return 0;
}

#endif
