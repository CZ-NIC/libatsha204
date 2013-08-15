#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

/**
 * Constants modifying Zone config parameter
 */
static const unsigned char IO_MEM_DATA = 2;
static const unsigned char IO_MEM_CONFIG = 0;
static const unsigned char IO_MEM_OTP = 1;
static const unsigned char IO_RW_ENC = 64;
static const unsigned char IO_RW_NON_ENC = 0;
static const unsigned char IO_RW_4_BYTES = 0;
static const unsigned char IO_RW_32_BYTES = 128; //10000000
static const unsigned char LOCK_CONFIG = 0;
static const unsigned char LOCK_DATA = 1;

/**
 * Generate zone config parameter (Param1)
 */
unsigned char get_zone_config(unsigned char io_mem, unsigned char io_enc, unsigned char io_cnt);

/**
 * Generate lock config parameter (Param1)
 */
unsigned char get_lock_config(unsigned char lock_what);
/**
 * Get slot address according to slot number.
 * Slot number is in range 0--15.
 */
unsigned char get_slot_address(unsigned char slot_number);
/**
 * Get slot config address according to slot number.
 * Slot number is in range 0--15.
 */
unsigned char get_slot_config_address(unsigned char slot_number);
unsigned char get_serial_number_address();
/**
 * Decode and get slot config word from raw data.
 * Slot number is in range 0--15.
 */
uint16_t decode_slot_config(unsigned char slot_number, unsigned char *data);

/**
 * Get device revision
 */
unsigned char *op_dev_rev();
uint32_t op_dev_rev_recv(unsigned char *packet);

/**
 * Let device generate random number
 */
unsigned char *op_random();
int op_random_recv(unsigned char *packet, unsigned char *data);

/**
 * Raw read and write to device
 */
unsigned char *op_raw_read(unsigned char zone_config, unsigned char address);
int op_raw_read_recv(unsigned char *packet, unsigned char *data);
unsigned char *op_raw_write(unsigned char zone_config, unsigned char address, size_t cnt, unsigned char *data);
int op_raw_write_recv(unsigned char *packet);
unsigned char *op_nonce(size_t cnt, unsigned char *data);
int op_nonce_recv(unsigned char *packet);
unsigned char *op_hmac(unsigned char address, bool use_sn_in_digest);
int op_hmac_recv(unsigned char *packet, unsigned char *data);
unsigned char *op_mac(unsigned char address, size_t cnt, unsigned char *data, bool use_sn_in_digest);
int op_mac_recv(unsigned char *packet, unsigned char *data);
unsigned char *op_serial_number();
int op_serial_number_recv(unsigned char *packet, unsigned char *data);
unsigned char *op_lock(unsigned char lock_config, const unsigned char *crc);
int op_lock_recv(unsigned char *packet);

#endif //OPERATIONS_H
