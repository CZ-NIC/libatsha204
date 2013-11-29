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

#include "atsha204.h"

const char *atsha_error_name(int err) {
	switch (err) {
		case ATSHA_ERR_OK:
			return "OK";

		case ATSHA_ERR_MEMORY_ALLOCATION_ERROR:
			return "Memory allocation error. Some memory allocation failed.";

		case ATSHA_ERR_INVALID_INPUT:
			return "Invalid input parameter.";

		case ATSHA_ERR_COMMUNICATION:
			return "Communication error: is not possible to send packet to the device, receive packet from the device, or multiple times was delivered/received malformed packet.";

		case ATSHA_ERR_BAD_COMMUNICATION_STATUS:
			return "Communication crashed due to bad status code from ATSHA204 device. Please, use verbose mode for more information.";

		case ATSHA_ERR_NOT_IMPLEMENTED:
			return "Operation not implemented. Requested function or feature in not implemented in applicable layer (yet).";

		case ATSHA_ERR_CONFIG_FILE_BAD_FORMAT:
			return "Configuration file has bad formatting.";

		case ATSHA_ERR_DNS_GET_KEY:
			return "Information about slot id couldn't be obtained from DNS.";

		case ATSHA_ERR_WAKE_NOT_CONFIRMED:
			return "Is not confirmed if device is wake up or not.";

		default:
			return "Error code is not in the list";
	}
}
