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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "atsha204consts.h"

#define TRY_SEND_RECV_ON_COMM_ERROR 5
#define TRY_SEND_RECV_ON_COMM_ERROR_TOUT 2000000
										//in microseconds (2s)
#define BUFFSIZE_NI2C ATSHA204_IO_BUFFER
#define BUFFSIZE_DNS 512
#define BUFFSIZE_LINE 128
#define BUFFSIZE_DNSMAGIC_ERRSTRLEN 1024

#define USE_OUR_SN true

#ifndef DEFAULT_EMULATION_CONFIG_PATH
#define DEFAULT_EMULATION_CONFIG_PATH "atsha204.sw"
#endif

#define NI2C_DEV_PATH_TURRIS "/dev/i2c-0"
#define NI2C_DEV_PATH_OMNIA "/dev/i2c-6"

#ifndef DEFAULT_NI2C_DEV_PATH
#define DEFAULT_NI2C_DEV_PATH NI2C_DEV_PATH_TURRIS
#endif

#ifndef DEFAULT_NI2C_ADDRESS
#define DEFAULT_NI2C_ADDRESS 0x64
#endif

#define DEFAULT_USE_SN_IN_DIGEST true
#define DEFAULT_DNS_RECORD_FIND_KEY "atsha-key.turris.cz"
#define DEFAULT_DNSSEC_ROOT_KEY "/etc/unbound/root.key"
#define LOCK_FILE "/tmp/libatsha204.lock"
#define LOCK_TRY_TOUT 10000
#define LOCK_TRY_MAX 2.2

#define USE_LAYER_EMULATION 0
#define USE_LAYER_NI2C 1
#ifndef USE_LAYER
#define USE_LAYER USE_LAYER_NI2C
#endif

#endif //CONFIGURATION_H
