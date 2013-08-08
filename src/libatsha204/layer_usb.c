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

static const int USB_PACKET_SKIP_PREFIX = 3;

/**
 * USB chip is returning bytes as string.
 * This function gets received message and return raw packet.
 * Function passes char by char and each pair is converting to one real byte
 */
static unsigned char *usb_get_raw_packet(char* data) {
	int high_cnt = 0, low_cnt = 1;
	unsigned char packet_size = get_number_from_hex_char(data[USB_PACKET_SKIP_PREFIX + high_cnt], data[USB_PACKET_SKIP_PREFIX + low_cnt]);

	if (packet_size == 0) {
		return NULL;
	}

	unsigned char *packet = (unsigned char *)calloc(packet_size, sizeof(unsigned char));
	if (packet == NULL) return NULL;
	packet[0] = packet_size;
	for (size_t i = 1; i < packet_size; i++) {
		high_cnt += 2;
		low_cnt += 2;
		packet[i] = get_number_from_hex_char(data[USB_PACKET_SKIP_PREFIX + high_cnt], data[USB_PACKET_SKIP_PREFIX + low_cnt]);
	}

	return packet;
}

static bool usb_check_nl(char *buff, size_t check_len) {
	for (size_t i = 0; i < check_len; i++) {
		if (buff[i] == '\n') {
			return true;
		}
	}

	return false;
}

static int usb_read(int dev, char *buff) {
	size_t check_len = 0, cnt = 0;


	while (!usb_check_nl(buff, check_len)) {
		cnt = read(dev, (buff + cnt), BUFFSIZE_USB);
		check_len += cnt;
		if (cnt <= 0) {
			return ATSHA_ERR_COMMUNICATION;
		}
	}

	return ATSHA_ERR_OK;
}

int usb_wake(int dev, unsigned char **answer) {
	char buff[BUFFSIZE_USB];
	clear_buffer((unsigned char *)buff, BUFFSIZE_USB);
	size_t len, cnt;
	int status;

	//Create messge
	strcpy(buff, "sha:wake()\n");

	//Send message
	len = strlen(buff);
	cnt = write(dev, buff, len);
	if (cnt != len) {
		return ATSHA_ERR_COMMUNICATION;
	}

	//Read answer
	clear_buffer((unsigned char *)buff, len);
	status = usb_read(dev, buff);
	if (status != ATSHA_ERR_OK) return status;

	if (buff[0] != '0' && buff[1] != '0') {
		if (g_config.verbose) log_message("ERR: Read packet: Malformed packet.");
		return ATSHA_ERR_COMMUNICATION;
	}

	//"Parse" packet from recieved message
	*answer = usb_get_raw_packet(buff);
	if (*answer == NULL) {
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	return ATSHA_ERR_OK;
}

int usb_idle(int dev) {
	char buff[BUFFSIZE_USB];
	clear_buffer((unsigned char *)buff, BUFFSIZE_USB);
	size_t len, cnt;
	int status;

	//Create messge
	strcpy(buff, "sha:idle()\n");

	//Send message
	len = strlen(buff);
	cnt = write(dev, buff, len);
	if (cnt != len) {
		return ATSHA_ERR_COMMUNICATION;
	}

	//Read answer
	clear_buffer((unsigned char *)buff, len);
	status = usb_read(dev, buff);
	if (status != ATSHA_ERR_OK) return status;

	if (strcmp(buff, "00()\n") != 0) {
		return ATSHA_ERR_USBCMD_NOT_CONFIRMED;
	}

	return ATSHA_ERR_OK;
}

int usb_command(int dev, unsigned char *raw_packet, unsigned char **answer) {
	char buff[BUFFSIZE_USB];
	clear_buffer((unsigned char *)buff, BUFFSIZE_USB);
	size_t len, cnt;
	int status;

	//Create messge
	strcpy(buff, "sha:talk(");
	int offset = 9;
	for (unsigned char i = 0; i < raw_packet[0]; i++) {
		sprintf((buff + offset), "%02x", raw_packet[i]);
		offset += 2;
	}
	strcpy((buff+offset), ")\n");

	//Send message
	len = strlen(buff);
	cnt = write(dev, buff, len);
	if (cnt != len) {
		return ATSHA_ERR_COMMUNICATION;
	}

	//Read answer
	clear_buffer((unsigned char *)buff, len);
	status = usb_read(dev, buff);
	if (status != ATSHA_ERR_OK) return status;

	if (buff[0] != '0' && buff[1] != '0') {
		if (g_config.verbose) log_message("ERR: Read packet: Malformed packet.");
		return ATSHA_ERR_COMMUNICATION;
	}

	//"Parse" packet from recieved message
	*answer = usb_get_raw_packet(buff);
	if (answer == NULL) {
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	return ATSHA_ERR_OK;
}
