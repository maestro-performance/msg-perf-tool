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
	gru_cli_option_help("daemon", "D", "run as a daemon in the background");
	gru_cli_option_help("duration", "d", "runs for the specificied amount of time. It must be suffixed by 's', 'm', 'h' or 'd' (ie.: 1h15s, 10m)");
	gru_cli_option_help("log-level",
		"l",
		"runs in the given verbose (info, stat, debug, etc) level mode");
	gru_cli_option_help(
		"log-dir", "L", "a directory to save the logs (mandatory for --daemon)");
	gru_cli_option_help(
		"parallel-count", "p", "number of parallel connections to the broker (require a log directory for > 1)");

	gru_cli_option_help("size", "s", "message size (in bytes)");
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
			{"broker-url", required_argument, 0, 'b'},
			{"duration", required_argument, 0, 'd'},
			{"log-level", required_argument, 0, 'l'},
			{"log-dir", required_argument, 0, 'L'},
			{"parallel-count", required_argument, 0, 'p'},
			{"message-size", required_argument, 0, 's'},
			{"daemon", no_argument, 0, 'D'},
			{"maestro-url", required_argument, 0, 'm'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}};

		c = getopt_long(argc, argv, "b:d:l:L:p:s:Dm:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'b':
				options->uri = gru_uri_parse(optarg, &status);
				if (gru_status_error(&status)) {
					fprintf(stderr, "%s", status.message);

					options_destroy(&options);
					return EXIT_FAILURE;
				}
				break;
			case 'd':
				if (!gru_duration_parse(&options->duration, optarg)) {
					fprintf(stderr, "Invalid input duration: %s\n", optarg);

					options_destroy(&options);
					return EXIT_FAILURE;
				}
				break;
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				gru_logger_set_mininum(options->log_level);
				break;
			case 'p':
				options->parallel_count = (uint16_t) atoi(optarg);
				break;
			case 's':
				options->message_size = atoll(optarg);
				break;
			case 'L':
				options->logdir = strdup(optarg);
				if (!options->logdir) {
					fprintf(stderr, "Unable to create memory for the log dir setting\n");

					options_destroy(&options);
					return EXIT_FAILURE;
				}
				break;
			case 'D':
				options->daemon = true;
				break;
			case 'm':
				options->maestro_uri = gru_uri_parse(optarg, &status);
				if (gru_status_error(&status)) {
					fprintf(stderr, "%s", status.message);

					options_destroy(&options);
					return EXIT_FAILURE;
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

	if (options->logdir) {
		remap_log(options->logdir, "mpt-receiver", 0, getpid(), stderr, &status);
	}
	else {
		if (options->parallel_count > 1) {
			fprintf(stderr, "Multiple concurrent process require a log directory\n");

			return EXIT_FAILURE;
		}
	}

	vmsl_t vmsl = vmsl_init();

	if (!vmsl_assign_by_url(&options->uri, &vmsl)) {
		goto err_exit;
	}
	
	logger_t logger = gru_logger_get();

	logger(INFO, "Starting test");
	if (receiver_start(&vmsl, options) == 0) {
		logger(INFO, "Test execution with process ID %d finished successfully\n", getpid());

		options_destroy(&options);
		return EXIT_SUCCESS;
	}
	
	err_exit:
	logger(INFO, "Test execution with process ID %d finished with errors\n", getpid());
	
	options_destroy(&options);
	return EXIT_FAILURE;
}
