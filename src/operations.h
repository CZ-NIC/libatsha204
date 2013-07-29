#ifndef OPERATIONS_H
#define OPERATIONS_H

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


/**
 * Constants enabling addressing
 */
static const unsigned char SLOT_ADDRESSES[] = {
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
	0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
};

static const unsigned char SLOT_CONFIG_ADDRESSES[] = {
	0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08,
	0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C
};


/**
 * Generate zone config parameter (Param1)
 */
unsigned char get_zone_config(unsigned char io_mem, unsigned char io_enc, unsigned char io_cnt);
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
/**
 * Decode and get slot config word from raw data.
 * Slot number is in range 0--15.
 */
uint16_t get_slot_config_word(unsigned char slot_number, unsigned char *data);
/**
 * Get device revision
 */
unsigned char *op_dev_rev();
uint32_t op_dev_rev_recv(unsigned char *packet);

/**
 * Let device generate random number
 */
unsigned char *op_random();
int op_random_recv(unsigned char *packet, unsigned char **data);

/**
 * Raw read and write to device
 */
unsigned char *op_raw_read(unsigned char zone_config, unsigned char address);
int op_raw_read_recv(unsigned char *packet, unsigned char **data);
unsigned char *op_raw_write(unsigned char zone_config, unsigned char address, size_t cnt, unsigned char *data);
int op_raw_write_recv(unsigned char *packet);


#endif //OPERATIONS_H
