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
#include "sender_tune_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-sender", argv[0]);

	gru_cli_option_help("help", "h", "show this help");

	gru_cli_option_help("broker-url", "b", "broker-url to connect to");
	gru_cli_option_help("log-level", "l",
					 "runs in the given verbose (info, stat, debug, etc) level mode");
	gru_cli_option_help("log-dir", "L",
					 "a directory to save the logs (mandatory for --daemon)");

	gru_cli_option_help("size", "s", "message size (in bytes)");
}

int tune_main(int argc, char **argv) {
	int c = 0;
	int option_index = 0;

	if (argc < 2) {
		show_help(argv);

		return EXIT_FAILURE;
	}

	options_t *options = options_new();
	gru_status_t status = gru_status_new();

	if (!options) {
		return EXIT_FAILURE;
	}

	set_options_object(options);

	const char *apphome = gru_base_app_home("mpt");
	config_init(options, apphome, "mpt-sender.ini", &status);
	if (!status.code == GRU_SUCCESS) {
		fprintf(stderr, "%s\n", status.message);

		return EXIT_FAILURE;
	}

	gru_logger_set(gru_logger_default_printer);

	while (1) {

		static struct option long_options[] = {
			{"broker-url", true, 0, 'b'},
			{"log-level", true, 0, 'l'},
			{"log-dir", true, 0, 'L'},
			{"size", true, 0, 's'},
			{"help", false, 0, 'h'}, {0, 0, 0, 0}};

		c = getopt_long(argc, argv, "b:l:s:L:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'b':
				options->uri = gru_uri_parse(optarg, &status);
				if (status.code != GRU_SUCCESS) {
					fprintf(stderr, "%s", status.message);
					goto err_exit;
				}

				if (strcmp(options->uri.scheme, "mqtt") == 0) {
					fprintf(stderr, "MQTT tune is not supported at the moment\n");
					goto err_exit;
				}
				break;
			case 'c':
				options->count = strtol(optarg, NULL, 10);
				break;
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				gru_logger_set_mininum(options->log_level);
				break;
			case 'd':
				options->duration = gru_duration_from_minutes(atoi(optarg));
				break;
			case 's':
				options->message_size = atoi(optarg);
				break;
			case 'L':
				strncpy(options->logdir, optarg, sizeof(options->logdir) - 1);
				break;
			case 'h':
				show_help(argv);
				return EXIT_SUCCESS;
			default:
				printf("Invalid or missing option\n");
				show_help(argv);
				return EXIT_FAILURE;
		}
	}

	vmsl_t vmsl = vmsl_init();

	if (!vmsl_assign_by_url(&options->uri, &vmsl)) {
		goto err_exit;
	}

	logger_t logger = gru_logger_get();

	tune_start(&vmsl, options);

	logger(INFO, "Tune execution terminated successfully");

success_exit:
	options_destroy(&options);
	return EXIT_SUCCESS;

err_exit:
	options_destroy(&options);
	return EXIT_FAILURE;
}
