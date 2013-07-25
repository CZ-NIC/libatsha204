#include "error.h"

const char *error_name(int err) {
	if (err == ERR_OK) {
		return "OK";
	} else if (err == ERR_MEMORY_ALLOCATION_ERROR) {
		return "Memory allocation error. Some memory allocation failed.";
	} else if (err == ERR_COMMUNICATION) {
		return "Communication error: is not possible to send packet to the device, receive packet from the device, or multiple times was delivered/received malformed packet.";
	} else if (err == ERR_WAKE_NOT_CONFIRMED) {
		return "Is not confirmed if device is wake up or not";
	} else {
		return "Error code is not in the list";
	}
}
