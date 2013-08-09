#include <stdio.h>
#include <stdint.h>

#include "../libatsha204/atsha204.h"
//#include "../libatsha204/mpsse.h"

void testing_log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

int main(void) {
	atsha_set_verbose();
	atsha_set_log_callback(testing_log_callback);

	//struct atsha_handle *handle = atsha_open_usb_dev(argv[1]);
	struct atsha_handle *handle = atsha_open_i2c_dev();
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open i2c devidce.\n");
		return 1;
	}

	int status;

	// Get Revision
	fprintf(stderr, "Get revision:\n");
	uint32_t rev = 0;
	status = atsha_dev_rev(handle, &rev);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	fprintf(stderr, "Revision: %u\n", rev);

	// Random number
	fprintf(stderr, "Random number:\n");
	atsha_big_int number;
	status = atsha_random(handle, &number);
	fprintf(stderr, "Status: %s\n", atsha_error_name(status));
	if (status == ATSHA_ERR_OK) {
		fprintf(stderr, "%zu bytes number: ", number.bytes); for (size_t i = 0; i < number.bytes; i++) { printf("%02X ", number.data[i]); } printf("\n");
	}

	return 0;
}
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
