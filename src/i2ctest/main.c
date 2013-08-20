#include <stdio.h>
#include <stdint.h>

#include "../libatsha204/atsha204.h"

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
