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
#include "receiver_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-receiver", argv[0]);

	gru_cli_option_help("help", "h", "show this help");

	gru_cli_option_help("broker-url", "b", "broker URL to connect to");
	gru_cli_option_help("count", "c", "sends a fixed number of messages");
	gru_cli_option_help("duration",
		"d",
		"runs for the specified amount of time. It "
		"must be suffixed by 's', 'm', 'h' or 'd' (ie.: "
		"1h15s, 10m)");
	gru_cli_option_help("log-level",
		"l",
		"runs in the given verbose (info, stat, debug, etc) level mode");
	gru_cli_option_help(
		"log-dir", "L", "a directory to save the logs (mandatory for --daemon)");
	gru_cli_option_help("parallel-count",
		"p",
		"number of parallel connections to the broker (require a log directory for > 1)");

	gru_cli_option_help("size", "s", "message size (in bytes)");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
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
		static struct option long_options[] = {{"broker-url", required_argument, 0, 'b'},
			{"duration", required_argument, 0, 'd'},
			{"log-level", required_argument, 0, 'l'},
			{"log-dir", required_argument, 0, 'L'},
			{"parallel-count", required_argument, 0, 'p'},
			{"message-size", required_argument, 0, 's'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}};

		int c = getopt_long(argc, argv, "b:d:l:L:p:s:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'b':
				if (!options_set_broker_uri(options, optarg, &status)) {
					fprintf(stderr, "%s\n", status.message);

					goto err_exit_0;
				}
				break;
			case 'd':
				if (!options_set_duration(options, optarg)) {
					fprintf(stderr, "Invalid duration: %s\n", optarg);

					goto err_exit_0;
				}
				break;
			case 'l':
                options_set_log_level(options, optarg);
				break;
			case 'p':
				options_set_parallel_count(options, optarg);
				break;
			case 's':
                options_set_message_size(options, optarg);
				break;
			case 'L':
				if (!options_set_logdir(options, optarg)) {
					fprintf(stderr, "Unable to allocate memory for setting the log directory\n");

					goto err_exit_0;
				}
				break;
			case 'm':
				if (!options_set_maestro_uri(options, optarg, &status)) {
					fprintf(stderr, "%s\n", status.message);

					goto err_exit_0;
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

				return EXIT_FAILURE;
		}
	}

	if (options_get_log_dir()) {
		remap_log_with_link(options_get_log_dir(), "mpt-receiver", 0, getpid(), stderr, &status);
	} else {
		if (options_get_parallel_count() > 1) {
			fprintf(stderr, "Multiple concurrent process require a log directory\n");

			goto err_exit_0;
		}
	}

	logger_t logger = gru_logger_get();
	vmsl_t vmsl = vmsl_init();

    const gru_uri_t broker_uri = options_get_broker_uri();

	if (!vmsl_assign_by_url(&broker_uri, &vmsl)) {
		goto err_exit_1;
	}

	logger(GRU_INFO, "Starting test");
	if (receiver_start(&vmsl, options) == 0) {
		logger(GRU_INFO, "Test execution with process ID %d finished successfully\n", getpid());

		options_destroy(&options);
		return EXIT_SUCCESS;
	}

err_exit_1:
	logger(GRU_INFO, "Test execution with process ID %d finished with errors\n", getpid());

err_exit_0:
	options_destroy(&options);
	return EXIT_FAILURE;
}
