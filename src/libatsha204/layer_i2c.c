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
#include "mpsse.h"

extern atsha_configuration g_config;

static void i2c_wait() {
	usleep(ATSHA204_I2C_CMD_TOUT);
}

int i2c_wake(struct atsha_handle *handle, unsigned char **answer) {
	int status;
	char wrwake[] = { 0x00 };
	char wraddr[] = { ATSHA204_I2C_ADDRESS } ;
	char *data;
log_message("WAKE");
	status = SetClock(handle->i2c, ATSHA204_I2C_WAKE_CLOCK); printf("Status: %d\n", status);
	status = Start(handle->i2c);
	status = Write(handle->i2c, wrwake, 1); printf("Status: %d\n", status);
	printf("ACK: %d\n", GetAck(handle->i2c));
	status = Stop(handle->i2c); printf("Status: %d\n", status);

	status = SetClock(handle->i2c, FOUR_HUNDRED_KHZ); printf("Status: %d\n", status);

	i2c_wait();

	status = Start(handle->i2c);

	wraddr[0] |= 1;
	status = Write(handle->i2c, wraddr, 1); printf("write addr Status: %d\n", status);
	printf("ACK: %d\n", GetAck(handle->i2c));
	data = Read(handle->i2c, BUFFSIZE_I2C); printf("read Status: %d\n", status);
	if (data == NULL) {
		log_message("layer_i2c: i2c_command: read data failed");
		return ATSHA_ERR_COMMUNICATION;
	}
	status = Stop(handle->i2c); printf("stop Status: %d\n", status);
	print_buffer_content(data, data[0]);

	*answer = calloc(data[0], sizeof(char));
	if (*answer == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	memcpy(*answer, data, data[0]);
	free(data);

	return ATSHA_ERR_OK;
}

int i2c_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	int status;
	char wraddr[] = { ATSHA204_I2C_ADDRESS } ;
	char *data;
log_message("COMMAND");
	status = Start(handle->i2c);
	status = Write(handle->i2c, wraddr, 1); printf("write addr Status: %d\n", status);
	printf("ACK: %d\n", GetAck(handle->i2c));
print_buffer_content(raw_packet, raw_packet[0]);
	status = Write(handle->i2c, raw_packet, raw_packet[0]); printf("write packet Status: %d\n", status);
	printf("ACK: %d\n", GetAck(handle->i2c));
	status = Stop(handle->i2c); printf("stop Status: %d\n", status);

	i2c_wait();

	status = Start(handle->i2c);

	wraddr[0] |= 1;
	status = Write(handle->i2c, wraddr, 1); printf("write addr Status: %d\n", status);
	printf("ACK: %d\n", GetAck(handle->i2c));
	data = Read(handle->i2c, BUFFSIZE_I2C); printf("read Status: %d\n", status);
	if (data == NULL) {
		log_message("layer_i2c: i2c_command: read data failed");
		return ATSHA_ERR_COMMUNICATION;
	}
	status = Stop(handle->i2c); printf("stop Status: %d\n", status);
print_buffer_content(data, data[0]);

	*answer = calloc(data[0], sizeof(char));
	if (*answer == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	memcpy(*answer, data, data[0]);
	free(data);

	return ATSHA_ERR_OK;
}
