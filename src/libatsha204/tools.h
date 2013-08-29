#ifndef TOOLS_H
#define TOOLS_H

#include<stdint.h>
#include<stdbool.h>

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
