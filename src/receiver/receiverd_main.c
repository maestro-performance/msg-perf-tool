/**
 *    Copyright 2017 Otavio Rodolfo Piske
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "receiverd_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-receiverd", argv[0]);

	gru_cli_option_help("help", "h", "show this help");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
}


int main(int argc, char **argv) {
	int c;
	int option_index = 0;

	options_t *options = options_new();
	set_options_object(options);

	gru_status_t status = gru_status_new();

	if (!options) {
		return EXIT_FAILURE;
	}

	if (argc < 2) {
		show_help(argv);

		return EXIT_FAILURE;
	}

	gru_logger_set(gru_logger_default_printer);

	while (1) {

		static struct option long_options[] = {
			{"log-level", true, 0, 'l'},
			{"log-dir", true, 0, 'L'},
			{"maestro-url", true, 0, 'm'},
			{"help", false, 0, 'h'},
			{0, 0, 0, 0}};

		c = getopt_long(argc, argv, "L:l:m:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				gru_logger_set_mininum(options->log_level);
				break;
			case 'L':
				strncpy(options->logdir, optarg, sizeof(options->logdir) - 1);
				break;
			case 'm':
				options->maestro_uri = gru_uri_parse(optarg, &status);
				if (gru_status_error(&status)) {
					fprintf(stderr, "%s", status.message);
					goto err_exit;
				}
				break;
			case 'h':
				show_help(argv);
				return EXIT_SUCCESS;
			default:
				printf("Invalid or missing option\n");
				show_help(argv);
				return EXIT_SUCCESS;
		}
	}

	init_controller(true, options->logdir, "mpt-receiver-daemon");

	// vmsl_t vmsl = vmsl_init();

	// if (!vmsl_assign_by_url(&options->uri, &vmsl)) {
	// 	goto err_exit;
	// }

	// int childs[5];
	// int child = 0;
	// logger_t logger = gru_logger_get();

	// if (options->parallel_count > 1) {
	// 	logger(INFO, "Creating %d concurrent operations", options->parallel_count);
	// 	for (uint16_t i = 0; i < options->parallel_count; i++) {
	// 		child = fork();

	// 		if (child == 0) {
	// 			if (strlen(options->logdir) > 0) {
	// 				remap_log(options->logdir,
	// 					"mpt-receiver",
	// 					getppid(),
	// 					getpid(),
	// 					stderr,
	// 					&status);
	// 			}

	// 			receiver_start(&vmsl, options);
	// 			goto success_exit;
	// 		} else {
	// 			if (child > 0) {
	// 				childs[i] = child;
	// 			} else {
	// 				printf("Error\n");
	// 			}
	// 		}
	// 	}

	// 	if (child > 0) {
	// 		int rc = 0;
	// 		for (uint16_t i = 0; i < options->parallel_count; i++) {
	// 			waitpid(childs[i], &rc, 0);

	// 			logger(INFO, "Child process %d terminated with status %d", childs[i], rc);
	// 		}
	// 	}
	// } else {
	// 	if (strlen(options->logdir) > 0 && options->daemon) {

	// 		remap_log(options->logdir, "mpt-receiver", 0, getpid(), stderr, &status);
	// 	}

	// 	logger(INFO, "Starting test");
	// 	receiver_start(&vmsl, options);
	// }

	// logger(INFO, "Test execution with parent ID %d terminated successfully\n", getpid());

success_exit:
	options_destroy(&options);
	return EXIT_SUCCESS;

err_exit:
	options_destroy(&options);
	return EXIT_FAILURE;
}