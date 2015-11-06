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

#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * \file tools.h
 * \brief Auxiliary tools used on different places
 */

/** \brief Calculates CRC.
 *
 * \param[in] length number of bytes in buffer
 * \param[in] data pointer to data for which CRC should be calculated
 * \param[out] crc pointer to 16-bit CRC
 */
void calculate_crc(uint16_t length, unsigned char *data, unsigned char *crc);

/**
 * \brief convert string that represents hexadecimal number to hexadecimal number
 *
 * \param high 4 most significant bytes of number
 * \param low 4 less significant bytes of number
 */
unsigned char get_number_from_hex_char(char high, char low);

/**
 * \brief Get unsigned 32bit integer from 4 bytes in memory
 *
 * \param data memory buffer containing at least 4 bytes
 */
uint32_t uint32_from_4_bytes(const unsigned char *data);

/**
 * \brief check memory block CRC against another CRC
 *
 * \param length of memory block
 * \param data memory block
 * \param another CRC
 */
bool check_crc(unsigned char length, unsigned char *data, unsigned char *crc);

/**
 * \brief Check if packet's CRC matches
 *
 * \param packet Raw packet
 */
bool check_packet(unsigned char *packet);

/**
 * \brief Packet generator
 *
 * This generator creates command packet. It is part of logic of middle layer.
 */
unsigned char *generate_command_packet(unsigned char opcode, unsigned char param1, uint16_t param2, unsigned char *data, unsigned char data_count);

/**
 * \brief Answer packet generator
 *
 * This generator creates packet with answer. It is part of emulation logic
 */
unsigned char *generate_answer_packet(unsigned char *data, unsigned char data_count);

/**
 * \brief Debug method
 */
void clear_buffer(unsigned char *buff, size_t len);
/**
 * \brief Debug method
 */
void print_buffer_content(unsigned char *buff, ssize_t len);

#endif //TOOLS_H
