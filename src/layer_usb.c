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

int usb_wake(int dev) {
	unsigned char buff[BUFFSIZE];
	clear_buffer(buff, BUFFSIZE);
	size_t len, cnt;

	strcpy((char *)buff, "sha:wake()");

	len = strlen((char *)buff);

	cnt = write(dev, buff, len);
	if (cnt != len) {
		return ERR_SEND;
	}

	clear_buffer(buff, len);
	cnt = read(dev, buff, BUFFSIZE);
	if (cnt <= 0) {
		return ERR_READ;
	}

	if (!check_packet(buff)) {
		return ERR_CRC;
	}

	if (buff[1] != ATSHA204_STATUS_WAKE_OK) {
		return ERR_WAKE_NOT_CONFIRMED;
	}

	return ERR_OK;
}
