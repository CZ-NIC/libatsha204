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
#include "configuration.h"
#include "atsha204.h"

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

static unsigned char *generate_command_packet(unsigned char opcode, unsigned char param1, uint16_t param2, unsigned char *data, unsigned char data_count) {
	unsigned char packet_size =
		1 + //count item
		1 + //opcode
		1 + //param1
		2 + //param2
		data_count + //data count
		2; //CRC

	unsigned char crc[2];
	unsigned char *packet = calloc(packet_size, sizeof(unsigned char));
	if (packet == NULL) {
		return NULL;
	}

	packet[0] = packet_size;
	packet[1] = opcode;
	packet[2] = param1;
	packet[3] = (param2 & 0x00FF);
	packet[4] = ((param2 & 0xFF00) >> 8);
	memcpy((packet + 5), data, data_count);
	calculate_crc(packet_size - 2, packet, crc); //skip crc slot
	packet[5 + data_count] = crc[0];
	packet[5 + data_count + 1] = crc[1];

	return packet;
}

//Should receive: 07 00 00 00 04 C0 2D
void op_dev_rev() {
	unsigned char *packet = generate_command_packet(ATSHA204_OPCODE_DEV_REV, 0, 00, NULL, 0);
	print_buffer_content(packet, packet[0]);
}
