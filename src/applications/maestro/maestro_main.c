/**
 Copyright 2016 Otavio Rodolfo Piske

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#include "maestro_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-maestro", argv[0]);

	gru_cli_option_help("help", "h", "show this help");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
	gru_cli_option_help("log-level",
		"l",
		"runs in the given verbose (info, debug, trace, etc) level mode");
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
	gru_logger_set(gru_logger_default_printer);

	while (1) {

		static struct option long_options[] = {{"maestro-url", required_argument, 0, 'm'},
			{"log-level", required_argument, 0, 'l'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}};

		int c = getopt_long(argc, argv, "m:l:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'm':
				if (!options_set_maestro_uri(options, optarg, &status)) {
					fprintf(stderr, "%s\n", status.message);

					goto err_exit;
				}
				break;
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				gru_logger_set_mininum(options->log_level);
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

	int child = fork();

	if (child == 0) {
		maestro_forward_daemon_run(options);
	} else {
		if (child > 0) {
			fprintf(stdout, "Forward daemon started\n");
			if (maestro_loop(&status) != 0) {
				goto err_exit;
			}

			kill(child, SIGTERM);
			int rc = 0;
			waitpid(child, &rc, 0);
		} else {
			fprintf(stderr, "Unable to launch child process");
			goto err_exit;
		}
	}

	options_destroy(&options);
	return EXIT_SUCCESS;

err_exit:
	options_destroy(&options);
	return EXIT_FAILURE;
}