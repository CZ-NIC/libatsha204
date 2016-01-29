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

#ifndef LIBATSHA204_H
#define LIBATSHA204_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**********************************************************************
 ******** THIS FILE REPRESENTS PUBLIC INTERFACE OF LIBATSHA204 *********
***********************************************************************/

/**
 * \file atsha204.h
 *
 * \brief This file represents public interface of libatsh204
 */

struct atsha_handle;

/**
 * \brief Maximum data size that could be used for communication with the chip.
 */
#define ATSHA_MAX_DATA_SIZE 32

/**
 * \brief Data structure for long numbers
 */
typedef struct {
	size_t bytes; ///<Length of transfered numbers
	unsigned char data[ATSHA_MAX_DATA_SIZE]; ///<Static buffer for data
} atsha_big_int;

//Library settings and initialization
/**
 * \brief Enable verbose mode
 */
void atsha_set_verbose();
/**
 * \brief Set callback for reporting errors and warnings from library
 */
void atsha_set_log_callback(void (*clb)(const char* msg));
/**
 * \brief Create instance of library. Let library to decide what kind of device is in the system.
 */
struct atsha_handle *atsha_open();
/**
 * \brief Create instance of library with chip on I2C bus driven by native kernel driver.
 * \return library instance hadler
 */
struct atsha_handle *atsha_open_ni2c_dev(const char *path, int address);
/**
 * \brief Create instance of library that emulates device
 * \param path Path to config file
 * \return library instance hadler
 */
struct atsha_handle *atsha_open_emulation(const char *path);
/**
 * \brief Create instance of library that emulates device for server-sice purposes
 * \param slot_id Slot ID of actual key of simulated device
 * \param serial_number Serial number of simulated device
 * \param key Actual key of simulated device
 * \return library instance hadler
 */
struct atsha_handle *atsha_open_server_emulation(unsigned char slot_id, const unsigned char *serial_number, const unsigned char *key);
/**
 * \brief Release all memory that library has allocated
 * \param handle Library instance
 */
void atsha_close(struct atsha_handle *handle);

/**
 * \brief Use DNS-Magic and find slot number for this device
 * \param handle Library instance
 * \return slot number of actual key-set specified for this device
 */
unsigned char atsha_find_slot_number(struct atsha_handle *handle);

/**
 * \brief Get chip DevRev number
 * \param handle Library instance
 * \param [out] revision revision number
 * \return status code
 */
int atsha_dev_rev(struct atsha_handle *handle, uint32_t *revision);
/**
 * \brief Let chip generate random number
 * \param handle Library instance
 * \param [out] random random number
 * \return status code
 */
int atsha_random(struct atsha_handle *handle, atsha_big_int *number);
/**
 * \brief Read content of slot, automatic version
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param [out] number returned key
 * \return status code
 */
int atsha_slot_read(struct atsha_handle *handle, atsha_big_int *number);
/**
 * \brief Read content of slot
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param slot_number Number of required slot
 * \param [out] number returned key
 * \return status code
 */
int atsha_raw_slot_read(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int *number);
/**
 * \brief Write key to slot, automatic version
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param number data to write
 * \return status code
 */
int atsha_slot_write(struct atsha_handle *handle, atsha_big_int number);
/**
 * \brief Write key to slot
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param slot_number Number of required slot
 * \param number data to write
 * \return status code
 */
int atsha_raw_slot_write(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int number);
//int atsha_slot_conf_read(struct atsha_handle *handle, unsigned char slot_number, uint16_t *config_word);
/**
 * \brief Perform the operation challenge-response, use HMAC algorithm, automatic version
 * \param handle Library instance
 * \param challenge Challenge data
 * \param [out] response Computed response
 * \return status code
 */
int atsha_challenge_response(struct atsha_handle *handle, atsha_big_int challenge, atsha_big_int *response);
/**
 * \brief Perform the operation challenge-response, use HMAC algorithm
 * \param handle Library instance
 * \param slot_number Number of required slot with key to combine with challenge
 * \param challenge Challenge data
 * \param [out] response Computed response
 * \param use_sn_in_digest Combine challenge with serial number
 * \return status code
 */
int atsha_low_challenge_response(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response, bool use_sn_in_digest);
/**
 * \brief Perform the operation challenge-response, use MAC algorithm, automatic version
 * \param handle Library instance
 * \param challenge Challenge data
 * \param [out] response Computed response
 * \return status code
 */
int atsha_challenge_response_mac(struct atsha_handle *handle, atsha_big_int challenge, atsha_big_int *response);
/**
 * \brief Perform the operation challenge-response, use MAC algorithm
 * \param handle Library instance
 * \param slot_number Number of required slot with key to combine with challenge
 * \param challenge Challenge data
 * \param [out] response Computed response
 * \param use_sn_in_digest Combine challenge with serial number
 * \return status code
 */
int atsha_low_challenge_response_mac(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response, bool use_sn_in_digest);
/**
 * \brief Get chip serial number defined by manufacturer
 * \param handle Library instance
 * \param [out] number serial number
 * \return status code
 */
int atsha_chip_serial_number(struct atsha_handle *handle, atsha_big_int *number);
/**
 * \brief Get chip serial number (depends on definition)
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param [out] number serial number
 * \return status code
 */
int atsha_serial_number(struct atsha_handle *handle, atsha_big_int *number);
/**
 * \brief Read data from configuration according to address
 * \param handle Library instance
 * \param address address in memory for applicable operation
 * \param [out] data retrieved data
 * \return status code
 */
int atsha_raw_conf_read(struct atsha_handle *handle, unsigned char address, atsha_big_int *data);
/**
 * \brief Write data to configuration according to address
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param address address in memory for applicable operation
 * \param [out] data data to write
 * \return status code
 */
int atsha_raw_conf_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data);
/**
 * \brief Read data from OTM memory according to address
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param address address in memory for applicable operation
 * \param [out] data retrieved data
 * \return status code
 */
int atsha_raw_otp_read(struct atsha_handle *handle, unsigned char address, atsha_big_int *data);
/**
 * \brief Write data to configuration according to address
 * \warning Success of this operation depends on actual state of the device and configuration of the slot
 * \param handle Library instance
 * \param address address in memory for applicable operation
 * \param [out] data data to write
 * \return status code
 */
int atsha_raw_otp_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data);
/**
 * \brief Get chip DevRev number
 * \param handle Library instance
 * \param [out] revision number
 * \return status code
 */
int atsha_lock_config(struct atsha_handle *handle, const unsigned char *crc);
/**
 * \brief Get chip DevRev number
 * \param handle Library instance
 * \param [out] revision number
 * \return status code
 */
int atsha_lock_data(struct atsha_handle *handle, const unsigned char *crc);

//Error management
#define ATSHA_ERR_OK 0
#define ATSHA_ERR_MEMORY_ALLOCATION_ERROR 1
#define ATSHA_ERR_INVALID_INPUT 2
#define ATSHA_ERR_COMMUNICATION 3
#define ATSHA_ERR_BAD_COMMUNICATION_STATUS 4
#define ATSHA_ERR_WAKE_NOT_CONFIRMED 5
#define ATSHA_ERR_NOT_IMPLEMENTED 6
#define ATSHA_ERR_CONFIG_FILE_BAD_FORMAT 7
#define ATSHA_ERR_DNS_GET_KEY 8

/**
 * \brief Get text description of error status code
 * \param err Status code
 * \return text description of error
 */
const char *atsha_error_name(int err);

#endif //LIBATSHA204_H
