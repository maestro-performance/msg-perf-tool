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
#include "brokerd_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-receiver-daemon", argv[0]);

	gru_cli_option_help("help", "h", "show this help");
	gru_cli_option_help("log-level",
		"l",
		"runs in the given verbose (info, stat, debug, etc) level mode");
	gru_cli_option_help(
		"log-dir", "L", "a directory to save the logs (mandatory for --daemon)");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
	gru_cli_option_help("name", "n", "the node name");
}

int main(int argc, char **argv) {
	int option_index = 0;

	if (argc < 2) {
		show_help(argv);

		return EXIT_FAILURE;
	}

	gru_status_t status = gru_status_new();
	options_t *options = options_new(&status);
	if (!options) {
		fprintf(stderr, "Unable to create options object: %s", status.message);
		return EXIT_FAILURE;
	}

	set_options_object(options);
	gru_logger_set(gru_logger_timed_printer);

	while (1) {

		static struct option long_options[] = {{"log-level", required_argument, 0, 'l'},
			{"log-dir", required_argument, 0, 'L'},
			{"maestro-url", required_argument, 0, 'm'},
			{"name", required_argument, 0, 'n'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}};

		int c = getopt_long(argc, argv, "l:L:m:n:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'l':
				options_set_log_level(options, optarg);
				break;
			case 'L':
				if (!options_set_logdir(options, optarg)) {
					fprintf(stderr, "Unable to allocate memory for setting the log directory\n");

					goto err_exit;
				}
				break;
			case 'm':
				if (!options_set_maestro_uri(options, optarg, &status)) {
					fprintf(stderr, "%s\n", status.message);

					goto err_exit;
				}
				break;
			case 'n':
				if (!options_set_name(options, optarg)) {
					fprintf(stderr, "Unable to allocate memory for setting the node name\n");

					goto err_exit;
				}
				break;
			case 'h':
				show_help(argv);
				options_destroy(&options);

				return EXIT_SUCCESS;
			default:
				printf("Invalid or missing option\n");
				show_help(argv);
				options_destroy(&options);

				return EXIT_SUCCESS;
		}
	}

	if (!options_get_log_dir()) {
		fprintf(stderr, "Log directory is mandatory for the receiver daemon\n");
		goto err_exit;
	}

	if (!gru_path_mkdirs(options_get_log_dir(), &status)) {
		fprintf(stderr, "Unable to create log directory: %s\n", status.message);
		goto err_exit;
	}

	if (!options_get_maestro_host()) {
		fprintf(stderr, "Maestro host is mandatory for the receiver daemon\n");
		goto err_exit;
	}

	int cret = init_controller(options_get_log_dir(), "mpt-broker-inspector");
	if (cret == 0) {
		if (brokerd_worker_start(options) != 0) {
			logger_t logger = gru_logger_get();
			logger(GRU_ERROR, "Unable to start the inspector daemon");

			goto err_exit;
		}

#ifdef __linux__
		fcloseall();
#endif

		options_destroy(&options);
		return EXIT_SUCCESS;
	}

	if (cret > 0) {
		options_destroy(&options);
		return EXIT_SUCCESS;
	}

err_exit:
	options_destroy(&options);
	return EXIT_FAILURE;
}