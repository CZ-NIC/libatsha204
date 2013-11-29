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

#ifndef ATSHA204CONSTS_H
#define ATSHA204CONSTS_H

/**
 * \file atsha204consts.h
 * \brief Constants that describe the device
 */

#define ATSHA204_NI2C_ADDRESS 0x64

#define ATSHA204_I2C_ADDRESS 0xC8
#define ATSHA204_I2C_WAKE_CLOCK 10000
#define ATSHA204_I2C_CMD_TOUT 100000
#define ATSHA204_I2C_WA_RESET 0x00
#define ATSHA204_I2C_WA_SLEEP 0x01
#define ATSHA204_I2C_WA_IDLE 0x02
#define ATSHA204_I2C_WA_COMMAND 0x03
#define ATSHA204_I2C_IO_ERR_RESPONSE 0xFF
#define ATSHA204_I2C_IO_ERR_RESPONSE_TRIES 5

//Status codes
#define ATSHA204_STATUS_SUCCES 0x00
#define ATSHA204_STATUS_CHECK_MAC_MISCOMPARE 0x01
#define ATSHA204_STATUS_PARSE_ERROR 0x03
#define ATSHA204_STATUS_EXEC_ERROR 0x0F
#define ATSHA204_STATUS_WAKE_OK 0x11
#define ATSHA204_STATUS_COMMUNICATION_ERROR 0xFF

//OpCodes
#define ATSHA204_OPCODE_DEV_REV 0x30
#define ATSHA204_OPCODE_RANDOM 0x1B
#define ATSHA204_OPCODE_READ 0x02
#define ATSHA204_OPCODE_WRITE 0x12
#define ATSHA204_OPCODE_NONCE 0x16
#define ATSHA204_OPCODE_HMAC 0x11
#define ATSHA204_OPCODE_MAC 0x08
#define ATSHA204_OPCODE_LOCK 0x17

//Sizes
#define ATSHA204_SN_BYTE_LEN 9
#define ATSHA204_SLOT_BYTE_LEN 32
#define ATSHA204_OTP_BYTE_LEN 4
#define ATSHA204_MAX_SLOT_NUMBER 15
#define ATSHA204_IO_BUFFER 84

//Public interface
#define ATSHA204_CONFIG_OTPMODE_READONLY 0xAA
#define ATSHA204_CONFIG_OTPMODE_LEGAY 0x00

//Our constants
#define ATSHA204_OTP_MEMORY_MAP_REV_NUMBER 0x00
#define ATSHA204_OTP_MEMORY_MAP_SERIAL_NUMBER 0x01
#define ATSHA204_OTP_MEMORY_MAP_ORIGIN_KEY_SET 0x02
#define ATSHA204_OTP_MEMORY_MAP_MAC_PREFIX 0x03
#define ATSHA204_OTP_MEMORY_MAP_MAC_ADDR  0x04


#endif //ATSHA204CONSTS_H
