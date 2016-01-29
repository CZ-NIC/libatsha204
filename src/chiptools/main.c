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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>
#include <stdlib.h>

#include "../libatsha204/atsha204.h"

#include "commands.h"

struct arguments {
	bool use_default;
	const char* path;
	int address;
	int commands_from;
};

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

static char doc[] = "ATSHA204(A) manipulation tool";
static char args_doc[] = "COMMAND";
static struct argp_option options[] = {
	{"default",				'd', 0, 0, "Use default values for bus and address", 0},
	{"address",				'a', "ADDR", 0, "Set address of ATSHA204(A)", 0},
	{"path",				'p', "STR", 0, "Set path of I2C BUS", 0},
	{ 0, 0, 0, 0, 0, 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	char *endptr;
	struct arguments *args = state->input;
	switch (key) {
	case 'd':
		args->use_default = true;
		break;
	case 'a':
		endptr = arg;
		args->address = strtol(arg, &endptr, 0);
		args->use_default = false;
		break;
	case 'p':
		args->path = arg;
		args->use_default = false;
		break;
	case ARGP_KEY_NO_ARGS:
		argp_usage(state);
		break;
	case ARGP_KEY_ARG:
		if (args->commands_from == 0) {
			args->commands_from = state->next - 1;
			state->next = state->argc;
		}
		break;
	//case ARGP_KEY_END:
	//	break;
	}

	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static void print_abi(atsha_big_int abi) {
	for (size_t i = 0; i < abi.bytes; i++) {
		printf("%02X ", abi.data[i]);
	}
	printf("\n");
}

int main(int argc, char **argv) {
	struct arguments args = (struct arguments) {
		.use_default = true,
		.address = 0,
		.path = NULL,
		.commands_from = 0
	};

	// Parse arguments
	argp_parse(&argp, argc, argv, 0, 0, &args);
	int cmdi = args.commands_from;
	const char *cmd = argv[cmdi];

	//init LIBATSHA204
	atsha_set_verbose();
	atsha_set_log_callback(log_callback);

	//Create LIBATSHA204 handler
	struct atsha_handle *handle = NULL;
	if (args.use_default) {
		handle = atsha_open();
	} else {
		handle = atsha_open_ni2c_dev(args.path, args.address);
	}
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open I2C devidce.\n");
		return 1;
	}

	atsha_big_int abi;

	if (strcmp(cmd, "dump-config") == 0) {
		dump_config(handle);

	} else if (strcmp(cmd, "dump-otp") == 0) {
		dump_otp(handle);

	} else if (strcmp(cmd, "dump-data") == 0) {
		dump_data(handle);

	} else if (strcmp(cmd, "sn") == 0) {
		if (atsha_serial_number(handle, &abi) == ATSHA_ERR_OK) {
			print_abi(abi);
		} else {
			return 2;
		}

	} else if (strcmp(cmd, "chipsn") == 0) {
		if (atsha_chip_serial_number(handle, &abi) == ATSHA_ERR_OK) {
			print_abi(abi);
		} else {
			return 2;
		}

	} else if (strcmp(cmd, "random") == 0) {
		if (atsha_random(handle, &abi) == ATSHA_ERR_OK) {
			print_abi(abi);
		} else {
			return 2;
		}

	} else if (strcmp(cmd, "slot") == 0) {
		printf("%d\n", atsha_find_slot_number(handle));


	} else if (strcmp(cmd, "compiled") == 0) {
		if (atsha_raw_slot_read(handle, 0, &abi) == ATSHA_ERR_OK) {
			print_abi(abi);
		} else {
			return 2;
		}

	} else {
		fprintf(stderr, "Undefined command\n");
		return 2;
	}

	atsha_close(handle);

	return 0;
}
