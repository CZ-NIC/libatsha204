#include<stdio.h>
#include<stdlib.h>
#include<string.h> //strerror()
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>

#include "atsha204.h"
#include "configuration.h"
#include "communication.h"
#include "tools.h"
#include "error.h"

static const int USB_PACKET_SKIP_PREFIX = 3;

/**
 * This function gets two chars, that are representing hex string, and
 * return real byte value.
 */
static unsigned char get_number_from_hex_char(char high, char low) {
	char str[2];
	str[0] = high; str[1] = low;

	return (unsigned char)strtol(str, NULL, 16);
}

/**
 * USB chip is returning bytes as string.
 * This function gets received message and return raw packet.
 * Function passes char by char and each pair is converting to one real byte
 */
static unsigned char *usb_get_raw_packet(char* data) {
	int high_cnt = 0, low_cnt = 1;
	unsigned char packet_size = get_number_from_hex_char(data[USB_PACKET_SKIP_PREFIX + high_cnt], data[USB_PACKET_SKIP_PREFIX + low_cnt]);

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

int usb_wake(int dev) {
	char buff[BUFFSIZE];
	clear_buffer((unsigned char *)buff, BUFFSIZE);
	size_t len, cnt;
	unsigned char *packet;

	//Create messge
	strcpy(buff, "sha:wake()\n");

	//Send message
	len = strlen(buff);
	cnt = write(dev, buff, len);
	if (cnt != len) {
		return ERR_COMMUNICATION;
	}

	//Read answer
	clear_buffer((unsigned char *)buff, len);
	cnt = read(dev, buff, BUFFSIZE);
	if (cnt <= 0) {
		return ERR_COMMUNICATION;
	}

	//"Parse" packet from recieved message
	packet = usb_get_raw_packet(buff);
	if (packet == NULL) {
		return ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Check packet consistency and check wake confirmation
	if (!check_packet(packet)) {
		free(packet);
		return ERR_COMMUNICATION;
	}

	if (packet[1] != ATSHA204_STATUS_WAKE_OK) {
		free(packet);
		return ERR_WAKE_NOT_CONFIRMED;
	}

	free(packet);
	return ERR_OK;
}

int usb_idle(int dev) {
	char buff[BUFFSIZE];
	clear_buffer((unsigned char *)buff, BUFFSIZE);
	size_t len, cnt;

	//Create messge
	strcpy(buff, "sha:idle()\n");

	//Send message
	len = strlen(buff);
	cnt = write(dev, buff, len);
	if (cnt != len) {
		return ERR_COMMUNICATION;
	}

	//Read answer
	clear_buffer((unsigned char *)buff, len);
	cnt = read(dev, buff, BUFFSIZE);
	if (cnt <= 0) {
		return ERR_COMMUNICATION;
	}

	if (strcmp(buff, "00()\n") != 0) {
		return ERR_USBCMD_NOT_CONFIRMED;
	}

	return ERR_OK;
}

int usb_command(int dev, unsigned char *raw_packet, unsigned char **answer) {
	char buff[BUFFSIZE];
	clear_buffer((unsigned char *)buff, BUFFSIZE);
	size_t len, cnt;

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
		return ERR_COMMUNICATION;
	}

	//Read answer
	clear_buffer((unsigned char *)buff, len);
	cnt = read(dev, buff, BUFFSIZE);
	if (cnt <= 0) {
		return ERR_COMMUNICATION;
	}

	//"Parse" packet from recieved message
	*answer = usb_get_raw_packet(buff);
	if (answer == NULL) {
		return ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Check packet consistency and check wake confirmation
	if (!check_packet(*answer)) {
		free(answer);
		return ERR_COMMUNICATION;
	}

	return ERR_OK;
}
