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
#include "maestro_loop.h"

static int maestro_loop_cmd(maestro_cmd_ctxt_t *cmd_ctxt, const char *cmd, gru_list_t *strings,
	gru_status_t *status)
{
	if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
		return -2;
	}

	if (strcmp(cmd, "start-receiver") == 0) {
		return maestro_cmd_start_receiver(cmd_ctxt, status);
	}

	if (strcmp(cmd, "stop-receiver") == 0) {
		return maestro_cmd_stop_receiver(cmd_ctxt, status);
	}

	if (strcmp(cmd, "start-sender") == 0) {
		return maestro_cmd_start_sender(cmd_ctxt, status);
	}

	if (strcmp(cmd, "stop-sender") == 0) {
		return maestro_cmd_stop_sender(cmd_ctxt, status);
	}

	if (strcmp(cmd, "start-inspector") == 0) {
		return maestro_cmd_start_inspector(cmd_ctxt, status);
	}

	if (strcmp(cmd, "stop-inspector") == 0) {
		return maestro_cmd_stop_inspector(cmd_ctxt, status);
	}

	if (strcmp(cmd, "start-all") == 0) {
		return maestro_cmd_start_all(cmd_ctxt, status);
	}

	if (strcmp(cmd, "stop-all") == 0) {
		return maestro_cmd_stop_all(cmd_ctxt, status);
	}

	if (strcmp(cmd, "collect") == 0) {
		return maestro_cmd_collect(cmd_ctxt, strings, status);
	}

	if (strcmp(cmd, "flush") == 0) {
		return maestro_cmd_flush(cmd_ctxt, status);
	}

	if (strcmp(cmd, "set") == 0) {
		return maestro_cmd_set_opt(cmd_ctxt, strings, status);
	}

	if (strcmp(cmd, "ping") == 0) {
		return maestro_cmd_ping(cmd_ctxt, status);
	}

	if (strcmp(cmd, "stats") == 0) {
		return maestro_cmd_stats(cmd_ctxt, strings, status);
	}

	if (strcmp(cmd, "halt") == 0) {
		return maestro_cmd_halt(cmd_ctxt, strings, status);
	}

	return -1;
}

int maestro_loop(gru_status_t *status) {

	const options_t *options = get_options_object();

	maestro_cmd_ctxt_t *cmd_ctxt = maestro_cmd_ctxt_init(&options->maestro_uri, status);
	if (!cmd_ctxt) {
		fprintf(stderr, "Unable to initialize command processor: %s\n", status->message);

		return 1;
	}

	gru_list_t *strings = NULL;
	do {
		char *raw_line = NULL;
		int ret = -1;

		raw_line = readline(RED "maestro" LIGHT_WHITE "> " RESET);
		if (raw_line == NULL) {
			break;
		}

		if (strlen(raw_line) == 0) {
			continue;
		}

		char *line = gru_trim(raw_line, strlen(raw_line));
		add_history(line);


		gru_split_clean(strings);
		gru_list_destroy(&strings);

		strings = gru_split(line, ' ', status);
		if (!strings) {
			fprintf(stderr, "Unable to split command: %s\n", status->message);
		}

		const gru_node_t *node = gru_list_get(strings, 0);
		char *command = gru_node_get_data_ptr(char, node);

		ret = maestro_loop_cmd(cmd_ctxt, command, strings, status);
		free(raw_line);

		if (ret == 0) {
			continue;
		} else if (ret == -1) {
			fprintf(stderr, "Unknown command: %s\n", command);
		} else if (ret == -2) {
			break;
		} else {
			fprintf(stderr, "%s\n", status->message);
		}
	} while (true);

	if (strings) {
		gru_split_clean(strings);
		gru_list_destroy(&strings);
	}

	maestro_cmd_ctxt_destroy(&cmd_ctxt);

	return 0;
}
