#ifndef TOOLS_H
#define TOOLS_H

#include<stdint.h>
#include<stdbool.h>

#define ERR(m) fprintf(stderr, "%s\n", (m))


/** \brief This function calculates CRC.
 *
 * \param[in] length number of bytes in buffer
 * \param[in] data pointer to data for which CRC should be calculated
 * \param[out] crc pointer to 16-bit CRC
 */
void calculate_crc(unsigned char length, unsigned char *data, unsigned char *crc);
bool check_crc(unsigned char length, unsigned char *data, unsigned char *crc);
void clear_buffer(unsigned char *buff, size_t len);
void print_buffer_content(unsigned char *buff, ssize_t len);

#endif //TOOLS_H
