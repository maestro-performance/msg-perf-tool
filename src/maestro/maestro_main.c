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
}

static char *start = "001";


int main(int argc, char **argv) {
	int c;
	int option_index = 0;

	options_t *options = options_new();
	gru_status_t status = gru_status_new();

	if (!options) {
		return EXIT_FAILURE;
	}

	if (argc < 2) {
		show_help(argv);

		return EXIT_FAILURE;
	}

	set_options_object(options);

	const char *apphome = gru_base_app_home("mpt");
	config_init(options, apphome, "mpt-maestro.ini", &status);
	if (gru_status_error(&status)) {
		fprintf(stderr, "%s\n", status.message);

		return EXIT_FAILURE;
	}

	gru_logger_set(gru_logger_default_printer);

	options->parallel_count = 1;
	while (1) {

		static struct option long_options[] = {
			{"maestro-url", true, 0, 'm'},
			{"help", false, 0, 'h'},
			{0, 0, 0, 0}};

		c = getopt_long(argc, argv, "m:h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
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
	
	vmsl_t vmsl = vmsl_init();

	if (!vmsl_assign_by_url(&options->maestro_uri, &vmsl)) {
		goto err_exit;
	}

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_BOTH, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_NONE,
	};

	opt.uri = options->maestro_uri;
	gru_uri_set_path(&opt.uri, "/mpt/receiver");

	msg_ctxt_t *ctxt = vmsl.init(NULL, opt, NULL, &status);

	if (!ctxt) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Failed to initialize maestro connection: %s", status.message);
		return 1;
	}

	vmsl.subscribe(ctxt, NULL, &status);
	if (!gru_status_success(&status)) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Failed to subscribe: %s", status.message);
		return 1;
	}

    char *command = NULL; 
	
	do  { 
		command = readline(RED "maestro " LIGHT_WHITE "> " RESET);
		if (command == NULL) {
			break;
		}

		add_history(command);
		if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
			break;
		}

		if (strcmp(command, "start") == 0) {
			msg_content_data_t req = {0};

			msg_content_data_init(&req, 3, NULL);
			req.data = strdup("001");
			req.size = 3;
			
			vmsl.send(ctxt, &req, &status);
		}

		
	} while (true);

success_exit:
	options_destroy(&options);
	return EXIT_SUCCESS;

err_exit:
	options_destroy(&options);
	return EXIT_FAILURE;
}