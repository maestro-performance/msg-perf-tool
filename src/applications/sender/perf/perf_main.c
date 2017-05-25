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
	gru_cli_option_help("duration",
		"d",
		"runs for the specificied amount of time. It "
		"must be suffixed by 's', 'm', 'h' or 'd' (ie.: "
		"1h15s, 10m)");
	gru_cli_option_help("log-level",
		"l",
		"runs in the given verbose (info, stat, debug, etc) level mode");
	gru_cli_option_help(
		"log-dir", "L", "a directory to save the logs (mandatory for --daemon)");
	gru_cli_option_help("no-probes", "N", "disable probes");
	gru_cli_option_help("parallel-count",
		"p",
		"number of parallel connections to the broker (require a log directory for > 1)");

	gru_cli_option_help("size",
		"s",
		"message size (in bytes). Use ~ to set a 5%% variability to the message size");
	gru_cli_option_help("throttle",
		"t",
		"sets a fixed rate of messages (in messages per second per connection)");
	gru_cli_option_help("maestro-url", "m", "maestro URL to connect to");
	gru_cli_option_help("interface", "i", "network interface for the network probe");
	gru_cli_option_help(
		"probes", "P", "comma-separated list of probes to enable (default: net,bmic)");
}

int perf_main(int argc, char **argv) {
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
			{"count", required_argument, 0, 'c'},
			{"log-level", required_argument, 0, 'l'},
			{"parallel-count", required_argument, 0, 'p'},
			{"duration", required_argument, 0, 'd'},
			{"size", required_argument, 0, 's'},
			{"log-dir", required_argument, 0, 'L'},
			{"throttle", required_argument, 0, 't'},
			{"no-probes", no_argument, 0, 'N'},
			{"interface", required_argument, 0, 'i'},
			{"maestro-url", required_argument, 0, 'm'},
			{"probes", required_argument, 0, 'P'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}};

		int c = getopt_long(
			argc, argv, "b:c:l:p:d:s:L:t:Ni:m:P:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'b':
				if (!options_set_broker_uri(options, optarg, &status)) {
					fprintf(stderr, "%s\n", status.message);

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
			case 'p':
				options->parallel_count = (uint16_t) atoi(optarg);
				break;
			case 'd':
				if (!gru_duration_parse(&options->duration, optarg)) {
					fprintf(stderr, "Invalid duration: %s\n", optarg);

					goto err_exit;
				}
				break;
			case 's':
				if (optarg[0] == '~') {
					options->message_size = atoi(optarg + 1);

					options->variable_size = true;
				} else {
					options->message_size = atoi(optarg);
				}
				break;
			case 'L':
				if (!options_set_logdir(options, optarg)) {
					fprintf(stderr, "Unable to allocate memory for setting the log directory\n");

					goto err_exit;
				}
				break;
			case 't':
				options->throttle = atoi(optarg);
				break;
			case 'N':
				options->probing = false;
				break;
			case 'i':
				if (!options_set_iface(options, optarg)) {
					fprintf(stderr, "Unable to allocate memory for setting the probing interface\n");

					goto err_exit;
				}
				break;
			case 'P':
				if (!options_set_probes(options, optarg, &status)) {
					fprintf(stderr, "Unable to set probes: %s\n", status.message);

					goto err_exit;
				}
				break;
			case 'm':
				if (!options_set_maestro_uri(options, optarg, &status)) {
					fprintf(stderr, "%s\n", status.message);

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

				return EXIT_FAILURE;
		}
	}

	if (options->logdir) {
		remap_log(options->logdir, "mpt-sender", 0, getpid(), stderr, &status);
	} else {
		if (options->parallel_count > 1) {
			fprintf(stderr, "Multiple concurrent process require a log directory\n");

			goto err_exit;
		}

		if (options->probing) {
			fprintf(stderr, "Log directory is mandatory for running probes\n");

			goto err_exit;
		}
	}

	logger_t logger = gru_logger_get();
	vmsl_t vmsl = vmsl_init();

	if (!vmsl_assign_by_url(&options->uri, &vmsl)) {
		goto err_exit;
	}

#ifdef LINUX_BUILD
	probe_scheduler_start(&status);
#endif // LINUX_BUILD

	if (perf_worker_start(&vmsl, options) == 0) {
		logger(
			INFO, "Test execution with process ID %d finished successfully\n", getpid());

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
