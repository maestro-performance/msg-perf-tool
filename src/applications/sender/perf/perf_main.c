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
#include <network/gru_uri.h>

#include "perf_main.h"

static void show_help(char **argv) {
	gru_cli_program_usage("mpt-sender", argv[0]);

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
	gru_cli_option_help("no-probes", "N", "disable probes");
	gru_cli_option_help(
		"parallel-count", "p", "number of parallel connections to the broker");

	gru_cli_option_help("size", "s", "message size (in bytes)");
	gru_cli_option_help("throttle",
		"t",
		"sets a fixed rate of messages (in messages per second per connection)");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
	gru_cli_option_help("interface", "i", "network interface for the network probe");
	gru_cli_option_help("probes", "P", 
		"comma-separated list of probes to enable (default: net,bmic)");
}

int perf_main(int argc, char **argv) {
	int c = 0;
	int option_index = 0;

	if (argc < 2) {
		show_help(argv);

		return EXIT_FAILURE;
	}

	options_t *options = options_new();
	set_options_object(options);

	gru_status_t status = gru_status_new();

	if (!options) {
		return EXIT_FAILURE;
	}

	gru_logger_set(gru_logger_default_printer);

	while (1) {

		static struct option long_options[] = {
			{"broker-url", true, 0, 'b'},
			{"count", true, 0, 'c'},
			{"log-level", true, 0, 'l'},
			{"parallel-count", true, 0, 'p'},
			{"duration", true, 0, 'd'},
			{"size", true, 0, 's'},
			{"log-dir", true, 0, 'L'},
			{"throttle", true, 0, 't'},
			{"daemon", false, 0, 'D'},
			{"no-probes", false, 0, 'N'},
			{"interface", true, 0, 'i'},
			{"maestro-url", true, 0, 'm'},
			{"probes", true, 0, 'P'},
			{"help", false, 0, 'h'},
			{0, 0, 0, 0}};

		c = getopt_long(argc, argv, "b:c:l:p:d:s:L:t:DNi:m:h", long_options, &option_index);
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
			case 'c':
				options->count = strtol(optarg, NULL, 10);
				break;
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				gru_logger_set_mininum(options->log_level);
				break;
			case 'p':
				options->parallel_count = (uint16_t) atoi(optarg);
				break;
			case 'd':
				if (!gru_duration_parse(&options->duration, optarg)) {
					fprintf(stderr, "Invalid input duration: %s\n", optarg);

					options_destroy(&options);
					return EXIT_FAILURE;
				}
				break;
			case 's':
				options->message_size = atoi(optarg);
				break;
			case 'L':
				options->logdir = strdup(optarg);
				if (!options->logdir) {
					fprintf(stderr, "Unable to create memory for the log dir setting\n");

					options_destroy(&options);
					return EXIT_FAILURE;
				}
				break;
			case 't':
				options->throttle = atoi(optarg);
				break;
			case 'D':
				options->daemon = true;
				break;
			case 'N':
				options->probing = false;
				break;
			case 'i':
				options->iface = strdup(optarg);
				break;
			case 'P':
				if (options->probes) {
					gru_list_destroy(&options->probes);
				}
				options->probes = gru_split(optarg, ',', &status);
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
				return EXIT_FAILURE;
		}
	}

	// init_controller(options->daemon, options->logdir, "mpt-sender-controller");
	if (options->logdir) {
		remap_log(options->logdir, "mpt-sender", 0, getpid(), stderr, &status);
	}

	vmsl_t vmsl = vmsl_init();

	if (!vmsl_assign_by_url(&options->uri, &vmsl)) {
		goto err_exit;
	}

	logger_t logger = gru_logger_get();

#ifdef LINUX_BUILD
	probe_scheduler_start(&status);
#endif // LINUX_BUILD


	if (perf_worker_start(&vmsl, options) == 0) {
		logger(INFO, "Test execution with process ID %d finished successfully\n", getpid());

#ifdef LINUX_BUILD
		probe_scheduler_stop();
#endif // LINUX_BUILD

		options_destroy(&options);
		return EXIT_SUCCESS;
	}

#ifdef LINUX_BUILD
	probe_scheduler_stop();
#endif // LINUX_BUILD


	err_exit:
	logger(INFO, "Test execution with process ID %d finished with errors\n", getpid());

	options_destroy(&options);
	return EXIT_FAILURE;
}
