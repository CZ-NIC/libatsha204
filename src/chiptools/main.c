/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2013, 2016 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
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
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>
#include <stdlib.h>

#include "../libatsha204/atsha204.h"

#include "commands.h"
#include "init.h"
#include "test.h"

struct arguments {
	bool no_command_ok;
	bool use_default;
	bool commands_help;
	const char* path;
	int address;
	int commands_from;
};

enum command {
	UNDEFINED = 0,
	DUMP_CONFIG,
	DUMP_OTP,
	DUMP_DATA,
	SN,
	CHIPSN,
	RANDOM,
	GET_SLOT,
	INIT,
	TEST,
	SET_ADDRESS,
	DEV_REV,
	SLEEP
};

struct commands_item {
	enum command command;
	const char *name;
	const char *usage;
	const char *help;
};

struct cleanup_data {
	struct atsha_handle *handle;
};

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

static char doc[] = "ATSHA204(A) manipulation tool";
static char args_doc[] = "COMMAND";
static struct argp_option options[] = {
	{"default",				'd', 0, 0, "Use default values for bus and address", 0},
	{"address",				'a', "ADDR", 0, "Set address of ATSHA204(A)", 0},
	{"bus",					'b', "STR", 0, "Set path of I2C BUS", 0},
	{"commands",			'c', 0, 0, "Show help for available commands", 0},
	{ 0, 0, 0, 0, 0, 0 }
};

static struct commands_item commands[] = {
	{ DUMP_CONFIG, "dump-config", NULL, "Dump content of configuration." },
	{ DUMP_OTP, "dump-otp", NULL, "Dump content of OTP memory." },
	{ DUMP_DATA, "dump-data", NULL, "Dump content of main memory (fails on locked slots)." },
	{ SN, "sn", NULL, "Get serial number of device." },
	{ CHIPSN, "chipsn", NULL, "Get serial number of ATSHA204." },
	{ RANDOM, "random", NULL, "Generate random number in ATSHA204." },
	{ GET_SLOT, "get-slot", NULL, "Get current slot from DNS record." },
	{ INIT, "init", "[FILE]", "Initialize ATSHA204 with memory content defined FILE." },
	{ TEST, "test", "[FILE]", "Test content of ATSHA204 with expected values." },
	{ SET_ADDRESS, "set-address", "[NUMBER]", "Change address of ATSHA204 to NUMBER." },
	{ DEV_REV, "devrev", NULL, "Get revision number of chip." },
	{ SLEEP, "sleep", "[NUMBER]", "Open ATSHA204 and sleep for NUMBER seconds. For debug purpose only." },
	{ 0, 0, 0, 0 }
};

static void print_commands_help(FILE *file)
{
	fprintf(file, "Available commands:\n");
	for (size_t i = 0; commands[i].name != NULL; i++) {
		if (commands[i].usage) {;
			printf("%s %s - %s\n", commands[i].name, commands[i].usage, commands[i].help);
		} else {
			printf("%s - %s\n", commands[i].name, commands[i].help);
		}
	}
}

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
	case 'b':
		args->path = arg;
		args->use_default = false;
		break;
	case 'c':
		args->commands_help = true;
		args->no_command_ok = true;
		break;
	case ARGP_KEY_NO_ARGS:
		if (!args->no_command_ok) {
			argp_usage(state);
		}
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

static enum command get_command(const char *name)
{
	for (size_t i = 0; commands[i].name != NULL; i++) {
		if (strcmp(commands[i].name, name) == 0) {
			return commands[i].command;
		}
	}

	return UNDEFINED;
}

static void print_abi(atsha_big_int abi) {
	for (size_t i = 0; i < abi.bytes; i++) {
		printf("%02X ", abi.data[i]);
	}
	printf("\n");
}

static long int parse_number(const char *str)
{
	char *endptr = (char *)str;
	long int tmp_number = strtol(str, &endptr, 0);

	return tmp_number;
}

static struct cleanup_data cleanup;

static void cleanup_on_exit(int status, void *arg)
{
	struct cleanup_data *data = (struct cleanup_data *) arg;

	// For better readability: in case of normal exit keep standard cleanup in main()
	if (status != 0) {
		atsha_close(data->handle);
	}
}

int main(int argc, char **argv) {
	struct arguments args = (struct arguments) {
		.no_command_ok = false,
		.use_default = true,
		.commands_help = false,
		.address = 0,
		.path = NULL,
		.commands_from = 0
	};

	// Parse arguments
	argp_parse(&argp, argc, argv, 0, 0, &args);
	int cmdi = args.commands_from;
	const char *cmd_name = argv[cmdi];

	if (args.commands_help) {
		print_commands_help(stdout);
		return 0;
	}

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

	cleanup.handle = handle;
	on_exit(cleanup_on_exit, &cleanup);

	// Process command
	atsha_big_int abi;
	int errcode = ATSHA_ERR_OK;
	bool command_failed = false;
	enum command command = get_command(cmd_name);
	switch (command) {
	case DUMP_CONFIG:
		if (!dump_config(handle)) {
			command_failed = true;
		}
		break;
	case DUMP_OTP:
		if (!dump_otp(handle)) {
			command_failed = true;
		}
		break;
	case DUMP_DATA:
		if (!dump_data(handle)) {
			command_failed = true;
		}
		break;
	case SN:
		if ((errcode = atsha_serial_number(handle, &abi)) == ATSHA_ERR_OK) {
			print_abi(abi);
		}
		break;
	case CHIPSN:
		if ((errcode = atsha_chip_serial_number(handle, &abi)) == ATSHA_ERR_OK) {
			print_abi(abi);
		}
		break;
	case RANDOM:
		if ((errcode = atsha_random(handle, &abi)) == ATSHA_ERR_OK) {
			print_abi(abi);
		}
		break;
	case GET_SLOT:
		printf("%d\n", atsha_find_slot_number(handle));
		break;
	case INIT:
		if (cmdi > argc - 2) {
			fprintf(stderr, "Bad argument count for command init\n\n");
			print_commands_help(stderr);
			return 2;
		}
		if (!init(handle, argv[cmdi + 1])) {
			command_failed = true;
		}
		break;
	case TEST:
		if (cmdi > argc - 2) {
			fprintf(stderr, "Bad argument count for command test\n\n");
			print_commands_help(stderr);
			return 2;
		}
		if (!test(handle, argv[cmdi + 1])) {
			command_failed = true;
		}
		break;
	case SET_ADDRESS:
		if (cmdi > argc - 2) {
			fprintf(stderr, "Bad argument count for command set-address\n\n");
			print_commands_help(stderr);
			return 2;
		}
		long int new_addr = parse_number(argv[cmdi + 1]);
		if (new_addr < 1 && new_addr > 0x7E) {
			fprintf(stderr, "Requested address is out of range\n");
			return 1;
		}

		errcode = atsha_change_address(handle, new_addr);
		if (errcode == ATSHA_ERR_OK) {
			printf("Address changed successfully\n");
		}
		break;
	case DEV_REV:
		// For next line thanks to compiler:
		// "a label can only be part of a statement and a declaration is not a statement"
		errcode = 0;
		uint32_t rev;
		if ((errcode = atsha_dev_rev(handle, &rev)) == ATSHA_ERR_OK) {
			printf("%" PRIu32 "\n", rev);
		}
		break;
	case SLEEP:
		if (cmdi > argc - 2) {
			fprintf(stderr, "Bad argument count for command sleep\n\n");
			print_commands_help(stderr);
			return 2;
		}
		long int seconds = parse_number(argv[cmdi + 1]);
		if (seconds < 1) {
			fprintf(stderr, "Sleep time should be a positive integer\n");
			return 1;
		}

		sleep(seconds);
		break;
	default:
		fprintf(stderr, "Undefined command\n\n");
		print_commands_help(stderr);
		return 2;
	}

	if (errcode != ATSHA_ERR_OK || command_failed) {
		fprintf(stderr, "Command failed\n");
		return 2;
	}

	// This is automatically closed in case of failure
	atsha_close(handle);

	return 0;
}
