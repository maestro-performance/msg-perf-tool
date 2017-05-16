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
#include "senderd_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-sender-daemon", argv[0]);

	gru_cli_option_help("help", "h", "show this help");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
	gru_cli_option_help("log-level",
		"l",
		"runs in the given verbose (info, stat, debug, etc) level mode");
	gru_cli_option_help(
		"log-dir", "L", "a directory to save the logs (mandatory for --daemon)");
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

		static struct option long_options[] = {{"log-level", required_argument, 0, 'l'},
			{"log-dir", required_argument, 0, 'L'},
			{"maestro-url", required_argument, 0, 'm'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}};

		c = getopt_long(argc, argv, "l:L:m:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				gru_logger_set_mininum(options->log_level);
				break;
			case 'L':
				options->logdir = strdup(optarg);
				if (!options->logdir) {
					fprintf(stderr, "Unable to create memory for the log dir setting\n");
					goto err_exit;
				}
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

	if (!options->logdir) {
		fprintf(stderr, "Log directory is mandatory for the sender daemon\n");
		goto err_exit;
	}

	if (!gru_path_mkdirs(options->logdir, &status)) {
		fprintf(stderr, "Unable to create log directory: %s\n", status.message);
		goto err_exit;
	}

	if (!options->maestro_uri.host) {
		fprintf(stderr, "Maestro host is mandatory for the sender daemon\n");
		goto err_exit;
	}

	init_controller(true, options->logdir, "mpt-sender-daemon");
	if (senderd_worker_start(options) != 0) {
		printf("Unable to start the sender worker\n");

		goto err_exit;
	}

	options_destroy(&options);
	return EXIT_SUCCESS;

err_exit:
	options_destroy(&options);
	return EXIT_FAILURE;
}