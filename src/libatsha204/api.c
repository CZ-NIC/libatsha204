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

#include <stdlib.h>
#include <unistd.h> //close()
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
//#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdarg.h>

#include "configuration.h"

#include "atsha204consts.h"
#include "atsha204.h"
#include "api.h"
#include "communication.h"
#include "tools.h"
#include "operations.h"
#include "tools.h"

/**
 * Global variable with configuration and some initial config values.
 */
atsha_configuration g_config = {
	.verbose = false,
	.log_callback = NULL
};

static const char *WARNING_WAKE_NOT_CONFIRMED = "WARNING: Device is possibly still awake";

void log_message(const char* msg, ...) {
	if (g_config.log_callback != NULL) {
		va_list vargs;
		va_start(vargs, msg);
		char *str = vaprintf(msg, vargs);
		va_end(vargs);

		g_config.log_callback(str);
	}
}

void atsha_set_verbose() {
	g_config.verbose = true;
}

void atsha_set_log_callback(void (*clb)(const char* msg)) {
	g_config.log_callback = clb;
}

/*
 * Try to open or create lockfile
 */
static int atsha_try_lock_file() {
	int lock;
	lock = open(LOCK_FILE, O_RDWR | O_CREAT, 0600 /* S_IRUSR | S_IWUSR, but these are not available on OpenWRT */);
	if (lock == -1) {
		log_message("api: try_lock: open lock file failed");
	}

	return lock;
}

/*
 * Attempts periodically to get lock. Time is limited.
 * When the time expires the operation fails.
 */
static bool atsha_lock(int lockfile) {
	int lock;
	double seconds;
	time_t now;
	time_t start = time(NULL);

	while (1) {
		lock = flock(lockfile, LOCK_EX | LOCK_NB);
		if (lock == -1) {
			now = time(NULL);
			seconds = difftime(now, start);
			if (seconds > LOCK_TRY_MAX) {
				log_message("api: atsha_lock: operation lock failed");
				return false;
			}
			usleep(LOCK_TRY_TOUT);
		} else {
			// Store PID of lock owner
			char *pid_str = aprintf("%d\n", getpid());
			int ret = write(lockfile, pid_str, strlen(pid_str));
			if (ret == -1) {
				log_message("api: atsha_lock: write of PID failed\n");
			} // Do not care about short reads. It is not common this code is just for debug purpose only.
			ret = fsync(lockfile);
			if (ret == -1) {
				log_message("api: atsha_lock: fsync failed\n");
			}

			return true;
		}
	}

	return true;
}

static void sanitize_lock(struct atsha_handle *handle) {
	handle->orig_alarm = alarm(KILL_UNRELEASED_LOCK_AFTER);
	int status = sigaction(SIGALRM, &(struct sigaction) { .sa_handler = SIG_DFL }, &(handle->orig_sigact));
	assert(status == 0);
}

static void restore_lock(struct atsha_handle *handle) {
	int status = sigaction(SIGALRM, &(handle->orig_sigact), NULL);
	assert(status == 0);
	alarm(handle->orig_alarm);
}

static void atsha_unlock(int lockfile) {
	//Return value of flock is not important - OS release it anyway
	flock(lockfile, LOCK_UN);
}

struct atsha_handle *atsha_open() {
	struct atsha_handle *handle;

#if USE_LAYER == USE_LAYER_NI2C
	handle = atsha_open_ni2c_dev((char *)DEFAULT_NI2C_DEV_PATH, DEFAULT_NI2C_ADDRESS);
#elif USE_LAYER == USE_LAYER_EMULATION
	handle = atsha_open_emulation((char *)DEFAULT_EMULATION_CONFIG_PATH);
#else
	fprintf(stderr, "Library was compiled without definition of bottom layer.\n");
	exit(1);
#endif

	return handle;
}

struct atsha_handle *atsha_open_ni2c_dev(const char *path, int address) {
	int try_lockfile = atsha_try_lock_file();
	if (try_lockfile == -1) {
		return NULL;
	}

	if (!atsha_lock(try_lockfile)) {
		close(try_lockfile);
		return NULL;
	}

	int try_fd = open(path, O_RDWR);
	if (try_fd == -1) {
		log_message("api: open_ni2c_dev: Couldn't open native I2C device.");
		return NULL;
	}

	if (ioctl(try_fd, I2C_SLAVE, address) < 0) {
		log_message("api: open_ni2c_dev: Couldn't bind address.");
		return NULL;
	}

	struct atsha_handle *handle = (struct atsha_handle *)calloc(1, sizeof(struct atsha_handle));
	if (handle == NULL) return NULL;

	handle->bottom_layer = BOTTOM_LAYER_NI2C;
	handle->is_srv_emulation = false;
	handle->fd = try_fd;
	handle->path = path;
	handle->addr = address;
	handle->file = NULL;
	handle->lockfile = try_lockfile;
	handle->i2c = NULL;
	handle->sn = NULL;
	handle->key = NULL;
	handle->key_origin = 0;
	handle->key_origin_cached = false;
	handle->slot_id = 0;
	handle->wake_is_expected = false;

	sanitize_lock(handle);

	return handle;
}

struct atsha_handle *atsha_open_emulation(const char *path) {
	if (path == NULL) return NULL;

	FILE *try_file = fopen(path, "r");
	if (try_file == NULL) {
		log_message("api: open_emulation: Couldn't open configuration file.");
		return NULL;
	}

	struct atsha_handle *handle = (struct atsha_handle *)calloc(1, sizeof(struct atsha_handle));
	if (handle == NULL) return NULL;

	handle->bottom_layer = BOTTOM_LAYER_EMULATION;
	handle->is_srv_emulation = false;
	handle->file = try_file;
	handle->lockfile = -1;
	handle->i2c = NULL;
	handle->sn = NULL;
	handle->key = NULL;
	handle->key_origin = 0;
	handle->key_origin_cached = false;
	handle->slot_id = 0;
	handle->wake_is_expected = false;

	atsha_big_int number;
	if (atsha_serial_number(handle, &number) != ATSHA_ERR_OK) {
		log_message("api: open_emulation: Couldn't read serial number.");
		atsha_close(handle);
		return NULL;
	}

	handle->sn = (unsigned char *)calloc(number.bytes, sizeof(unsigned char));
	if (handle->sn == NULL) {
		log_message("api: open_emulation: Copy SN memory allocation error");
		atsha_close(handle);
		return NULL;
	}
	memcpy(handle->sn, number.data, number.bytes);

	if (atsha_raw_otp_read(handle, ATSHA204_OTP_MEMORY_MAP_ORIGIN_KEY_SET, &number) != ATSHA_ERR_OK) {
		log_message("api: open_emulation: Couldn't read key origin");
		atsha_close(handle);
		return NULL;
	}

	handle->key_origin = uint32_from_4_bytes(number.data);
	handle->key_origin_cached = true;

	return handle;
}

struct atsha_handle *atsha_open_server_emulation(unsigned char slot_id, const unsigned char *serial_number, const unsigned char *key) {
	if (serial_number == NULL || key == NULL) return NULL;

	struct atsha_handle *handle = (struct atsha_handle *)calloc(1, sizeof(struct atsha_handle));
	if (handle == NULL) return NULL;

	handle->bottom_layer = BOTTOM_LAYER_EMULATION;
	handle->is_srv_emulation = true;
	handle->file = NULL;
	handle->lockfile = -1;
	handle->i2c = NULL;
	handle->key_origin = 0;
	handle->key_origin_cached = false;
	handle->slot_id = slot_id;

	if (USE_OUR_SN) {
		handle->sn = (unsigned char *)calloc(2*ATSHA204_OTP_BYTE_LEN, sizeof(unsigned char));
	} else {
		handle->sn = (unsigned char *)calloc(ATSHA204_SN_BYTE_LEN, sizeof(unsigned char));
	}
	if (handle->sn == NULL) return NULL;
	handle->key = (unsigned char *)calloc(ATSHA204_SLOT_BYTE_LEN, sizeof(unsigned char));
	if (handle->key == NULL) return NULL;

	if (USE_OUR_SN) {
		memcpy(handle->sn, serial_number, 2*ATSHA204_OTP_BYTE_LEN);
	} else {
		memcpy(handle->sn, serial_number, ATSHA204_SN_BYTE_LEN);
	}
	memcpy(handle->key, key, ATSHA204_SLOT_BYTE_LEN);

	return handle;
}

void atsha_close(struct atsha_handle *handle) {
	if (handle == NULL) return;

	if (handle->bottom_layer == BOTTOM_LAYER_NI2C) {
		close(handle->fd);
	}

	if (handle->file != NULL) {
		fclose(handle->file);
	}

	if (handle->lockfile != -1) {
		atsha_unlock(handle->lockfile);
		restore_lock(handle);
		close(handle->lockfile);
	}

	free(handle->sn);
	free(handle->key);

	free(handle);
}

int atsha_dev_rev(struct atsha_handle *handle, uint32_t *revision) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_dev_rev();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	*revision = op_dev_rev_recv(answer);

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_random(struct atsha_handle *handle, atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_random();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_random_recv(answer, number->data);
	if (number->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_slot_read(struct atsha_handle *handle, atsha_big_int *number) {
	unsigned char slot_number = atsha_find_slot_number(handle);
	if (slot_number == DNS_ERR_CONST) return ATSHA_ERR_DNS_GET_KEY;

	return atsha_raw_slot_read(handle, slot_number, number);
}

int atsha_raw_slot_read(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	if (slot_number > ATSHA204_MAX_SLOT_NUMBER) {
		log_message("api: low_slot_read: requested slot number is bigger than max slot number");
		return ATSHA_ERR_INVALID_INPUT;
	}

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_read(get_zone_config(IO_MEM_DATA, IO_RW_NON_ENC, IO_RW_32_BYTES), get_slot_address(slot_number));
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_raw_read_recv(answer, number->data);
	if (number->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_slot_write(struct atsha_handle *handle, atsha_big_int number) {
	unsigned char slot_number = atsha_find_slot_number(handle);
	if (slot_number == DNS_ERR_CONST) return ATSHA_ERR_DNS_GET_KEY;

	return atsha_raw_slot_write(handle, slot_number, number);
}

int atsha_raw_slot_write(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	if (slot_number > ATSHA204_MAX_SLOT_NUMBER) {
		log_message("api: low_slot_write: requested slot number is bigger than max slot number");
		return ATSHA_ERR_INVALID_INPUT;
	}

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_write(get_zone_config(IO_MEM_DATA, IO_RW_NON_ENC, IO_RW_32_BYTES), get_slot_address(slot_number), number.bytes, number.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_raw_write_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_challenge_response(struct atsha_handle *handle, atsha_big_int challenge, atsha_big_int *response) {
	unsigned char slot_number = atsha_find_slot_number(handle);
	if (slot_number == DNS_ERR_CONST) return ATSHA_ERR_DNS_GET_KEY;

	return atsha_low_challenge_response(handle, slot_number, challenge, response, DEFAULT_USE_SN_IN_DIGEST);
}

int atsha_low_challenge_response(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response, bool use_sn_in_digest) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	if (slot_number > ATSHA204_MAX_SLOT_NUMBER) {
		log_message("api: low_challenge_response: requested slot number is bigger than max slot number");
		return ATSHA_ERR_INVALID_INPUT;
	}
	if (challenge.bytes != 32) {
		log_message("api: low_challenge_response: challnege is bigger than 32 bytes");
		return ATSHA_ERR_INVALID_INPUT;
	}

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	//Store Challenge to TempKey memory
	////////////////////////////////////////////////////////////////////
	packet = op_nonce(challenge.bytes, challenge.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_nonce_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Clean and reinit
	free(packet);
	free(answer);
	answer = NULL;

	//Get HMAC digest
	////////////////////////////////////////////////////////////////////
	packet = op_hmac(slot_number, use_sn_in_digest);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	response->bytes = op_hmac_recv(answer, response->data);
	if (response->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_challenge_response_mac(struct atsha_handle *handle, atsha_big_int challenge, atsha_big_int *response) {
	unsigned char slot_number = atsha_find_slot_number(handle);
	if (slot_number == DNS_ERR_CONST) return ATSHA_ERR_DNS_GET_KEY;

	return atsha_low_challenge_response_mac(handle, slot_number, challenge, response, DEFAULT_USE_SN_IN_DIGEST);
}

int atsha_low_challenge_response_mac(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response, bool use_sn_in_digest) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	if (slot_number > ATSHA204_MAX_SLOT_NUMBER) {
		log_message("api: low_challenge_response_mac: requested slot number is bigger than max slot number");
		return ATSHA_ERR_INVALID_INPUT;
	}

	if (challenge.bytes != 32) {
		log_message("api: low_challenge_response_max: challenge is bigger than 32 bytes");
		return ATSHA_ERR_INVALID_INPUT;
	}

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	//Store Challenge to TempKey memory
	////////////////////////////////////////////////////////////////////
	packet = op_mac(slot_number, challenge.bytes, challenge.data, use_sn_in_digest);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	response->bytes = op_mac_recv(answer, response->data);
	if (response->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_chip_serial_number(struct atsha_handle *handle, atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_serial_number();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_serial_number_recv(answer, number->data);
	if (number->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_serial_number(struct atsha_handle *handle, atsha_big_int *number) {
	if (USE_OUR_SN) {
		int status;
		atsha_big_int part;
		status = atsha_raw_otp_read(handle, ATSHA204_OTP_MEMORY_MAP_REV_NUMBER, &part);
		if (status != ATSHA_ERR_OK) return status;
		number->data[0] = part.data[0]; number->data[1] = part.data[1];
		number->data[2] = part.data[2]; number->data[3] = part.data[3];

		status = atsha_raw_otp_read(handle, ATSHA204_OTP_MEMORY_MAP_SERIAL_NUMBER, &part);
		if (status != ATSHA_ERR_OK) return status;
		number->data[4] = part.data[0]; number->data[5] = part.data[1];
		number->data[6] = part.data[2]; number->data[7] = part.data[3];

		number->bytes = 8;

		return ATSHA_ERR_OK;
	} else {
		return atsha_chip_serial_number(handle, number);
	}
}

int atsha_change_address(struct atsha_handle *handle, unsigned char address) {
	if (address < 1 && address > 0x7E) {
		return ATSHA_ERR_INVALID_INPUT;
	}

	int status;
	atsha_big_int config_line;

	status = atsha_raw_conf_read(handle, ATSHA204_CNF_MEMORY_MAP_I2C_ADDRESS, &config_line);
	if (status != ATSHA_ERR_OK) return status;

	config_line.data[0] = address << 1;
	status = atsha_raw_conf_write(handle, ATSHA204_CNF_MEMORY_MAP_I2C_ADDRESS, config_line);

	return status;
}

int atsha_raw_conf_read(struct atsha_handle *handle, unsigned char address, atsha_big_int *data) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_read(get_zone_config(IO_MEM_CONFIG, IO_RW_NON_ENC, IO_RW_4_BYTES), address);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	data->bytes = op_raw_read_recv(answer, data->data);
	if (data->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_raw_conf_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_write(get_zone_config(IO_MEM_CONFIG, IO_RW_NON_ENC, IO_RW_4_BYTES), address, data.bytes, data.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_raw_write_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_raw_otp_read(struct atsha_handle *handle, unsigned char address, atsha_big_int *data) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_read(get_zone_config(IO_MEM_OTP, IO_RW_NON_ENC, IO_RW_4_BYTES), address);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	data->bytes = op_raw_read_recv(answer, data->data);
	if (data->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_raw_otp_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_write(get_zone_config(IO_MEM_OTP, IO_RW_NON_ENC, IO_RW_4_BYTES), address, data.bytes, data.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_raw_write_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_raw_otp32_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	if (data.bytes != 32) {
		return ATSHA_ERR_INVALID_INPUT;
	}

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_write(get_zone_config(IO_MEM_OTP, IO_RW_NON_ENC, IO_RW_32_BYTES), address, data.bytes, data.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_raw_write_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_lock_config(struct atsha_handle *handle, const unsigned char *crc) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_lock(get_lock_config(LOCK_CONFIG), crc);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_lock_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_lock_data(struct atsha_handle *handle, const unsigned char *crc) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_lock(get_lock_config(LOCK_DATA), crc);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_lock_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}
