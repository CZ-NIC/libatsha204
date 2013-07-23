#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h> //strerror()
#include<stdint.h>
#include<stdbool.h>

#include "tools.h"

bool check_packet(unsigned char *packet) {
	unsigned char packet_size;
	unsigned char crc[2];

	packet_size = packet[0];
	crc[0] = packet[packet_size-2];
	crc[1] = packet[packet_size-1];

	if (!check_crc(packet_size-2, packet, crc)) {
		return false;
	}

	return true;
}

bool wake_chip() {

	return true;
}
