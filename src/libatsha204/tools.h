#ifndef TOOLS_H
#define TOOLS_H

#include<stdint.h>
#include<stdbool.h>

/** \brief This function calculates CRC.
 *
 * \param[in] length number of bytes in buffer
 * \param[in] data pointer to data for which CRC should be calculated
 * \param[out] crc pointer to 16-bit CRC
 */
void calculate_crc(unsigned char length, unsigned char *data, unsigned char *crc);
unsigned char get_number_from_hex_char(char high, char low);
bool check_crc(unsigned char length, unsigned char *data, unsigned char *crc);
void clear_buffer(unsigned char *buff, size_t len);
void print_buffer_content(unsigned char *buff, ssize_t len);
bool check_packet(unsigned char *packet);
unsigned char *generate_command_packet(unsigned char opcode, unsigned char param1, uint16_t param2, unsigned char *data, unsigned char data_count);
unsigned char *generate_answer_packet(unsigned char *data, unsigned char data_count);

#endif //TOOLS_H
