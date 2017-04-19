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

#include "maestro_command.h"


static int maestro_cmd_connect(maestro_cmd_ctxt_t *cmd_ctxt, gru_uri_t uri, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_NONE,
	};

	msg_conn_info_gen_id(&opt.conn_info);
	opt.uri = uri;
	
	cmd_ctxt->msg_ctxt = cmd_ctxt->vmsl.init(NULL, opt, NULL, status);

	if (!cmd_ctxt->msg_ctxt) {
		logger(ERROR, "Failed to initialize maestro connection: %s", status->message);
		return 1;
	}

	return 0;
}


static int maestro_cmd_disconnect(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	cmd_ctxt->vmsl.stop(cmd_ctxt->msg_ctxt, status);
	return 0;
}



int maestro_cmd_start_receiver(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	const options_t *options = get_options_object();

	int ret = maestro_cmd_connect(cmd_ctxt, options->maestro_uri, status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, "/mpt/receiver");
	
	msg_content_data_t req = {0};
	maestro_easy_request(&req, MAESTRO_NOTE_START);
	
	vmsl_stat_t rstat = cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, &req, status);
	if (rstat != VMSL_SUCCESS) {
		fprintf(stderr, "Failed to send command");

		return 1;
	}

	ret = maestro_cmd_disconnect(cmd_ctxt, status);
	if (ret != 0) {
		fprintf(stderr, "Warning error during disconnect");

		return 1;
	}

	return 0;
}

static void maestro_cmd_print_data(maestro_note_t *note) {
	if (maestro_note_equals(note, MAESTRO_NOTE_PING)) {
		printf("ID: %.*s Time: %.*s ms\n", 
			(int) sizeof(note->payload->response.ping.id), note->payload->response.ping.id, 
			(int) sizeof(note->payload->response.ping.elapsed), note->payload->response.ping.elapsed);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_PROTOCOL_ERROR)) {
		printf("One of more of the commands did not complete successfully\n");
	} else if (maestro_note_equals(note, MAESTRO_NOTE_OK)) {
		printf("Peer reply OK\n");
	}
}


int maestro_cmd_collect(maestro_cmd_ctxt_t *cmd_ctxt, int queue, gru_status_t *status) {
	ssize_t ret = 0;
	while (true) {
		char buf[MAESTRO_NOTE_SIZE] = {0};

		ret = msgrcv(queue, &buf, sizeof(buf), 0, IPC_NOWAIT);
		if (ret < 0) {
			if (errno == ENOMSG) { 
				break;
			}
			else {
				fprintf(stdout, "Failed to read from the local forward queue\n");
				
				return 1;
			}
		}
		else {
			maestro_note_t note = {0};

			if (!maestro_note_parse(buf, ret, &note, status)) {
				fprintf(stderr, "Unknown protocol data\n");
			}
			else {
				maestro_cmd_print_data(&note);
			}

			maestro_note_payload_cleanup(&note);
		}
	}

	return 0;
}


int maestro_cmd_flush(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	const options_t *options = get_options_object();

	int ret = maestro_cmd_connect(cmd_ctxt, options->maestro_uri, status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, "/mpt/receiver");
	
	msg_content_data_t req = {0};
	maestro_easy_request(&req, MAESTRO_NOTE_FLUSH);

	vmsl_stat_t rstat = cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, &req, status);
	if (rstat != VMSL_SUCCESS) {
		fprintf(stderr, "Failed to send command");
		return 1;
	}

	ret = maestro_cmd_disconnect(cmd_ctxt, status);
	if (ret != 0) {
		fprintf(stderr, "Warning error during disconnect");

		return 1;
	}

	return 0;
}


static int maestro_cmd_set_opt_by_name(msg_content_data_t *data, const char *opt, 
	const char *val, gru_status_t *status) {
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_REQUEST);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_SET);

	if (!maestro_note_payload_prepare(&note, status)) {
		return -1;
	}

	if (strcmp(opt, "broker") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_BROKER, val);
	} else if (strcmp(opt, "duration") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_DURATION_TYPE, val);
	} else if  (strcmp(opt, "log-level") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_LOG_LEVEL, val);
	} else if  (strcmp(opt, "parallel-count") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT, val);
	} else if  (strcmp(opt, "message-size") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE, val);
	} else if  (strcmp(opt, "throttle") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_THROTTLE, val);
	} else {
		return -1;
	}

	maestro_serialize_note(&note, data);
	maestro_note_payload_cleanup(&note);
	return 0;
}


int maestro_cmd_set_opt(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings, gru_status_t *status) {
	const options_t *options = get_options_object();


	const gru_node_t *node = gru_list_get(strings, 1);
	if (!node) {
		gru_status_set(status, GRU_FAILURE, "Missing option name");
		return 1;
	}

	char *opt = gru_node_get_data_ptr(char, node);

	if (!node->next) { 
		gru_status_set(status, GRU_FAILURE, "Missing option value: parser error");
		return 1;
	}
	
	char *val = gru_node_get_data_ptr(char, node->next);
	if (!val) {
		gru_status_set(status, GRU_FAILURE, "Missing option value: invalid CLI data");
		return 1;
	}

	int ret = maestro_cmd_connect(cmd_ctxt, options->maestro_uri, status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, "/mpt/receiver");
	
	msg_content_data_t req = {0};
	if (maestro_cmd_set_opt_by_name(&req, opt, val, status) == -1)  {
		gru_status_set(status, GRU_FAILURE, "Invalid option: %s", opt);

		return 1;
	}

	vmsl_stat_t rstat = cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, &req, status);
	if (rstat != VMSL_SUCCESS) {
		fprintf(stderr, "Failed to send command");
		return 1;
	}

	ret = maestro_cmd_disconnect(cmd_ctxt, status);
	if (ret != 0) {
		fprintf(stderr, "Warning error during disconnect");
	}

	return 0;
}

static void maestro_cmd_request_ping(msg_content_data_t *out, gru_status_t *status) {
	maestro_note_t note = {0};

	if (!maestro_note_payload_prepare(&note, status)) {
		return;
	}

	maestro_note_set_type(&note, MAESTRO_TYPE_REQUEST);
	maestro_note_set_cmd(&note, MAESTRO_NOTE_PING);

	gru_timestamp_t ts = gru_time_now();
	char *formatted_ts = gru_time_write_str(&ts);
	maestro_note_ping_set_ts(&note, formatted_ts);

	maestro_serialize_note(&note, out);

	gru_dealloc_string(&formatted_ts);
	maestro_note_payload_cleanup(&note);
}

int maestro_cmd_ping(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	const options_t *options = get_options_object();

	int ret = maestro_cmd_connect(cmd_ctxt, options->maestro_uri, status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, "/mpt/receiver");
	
	msg_content_data_t req = {0};
	maestro_cmd_request_ping(&req, status);
	if (!gru_status_success(status)) {
		return 1;
	}

	vmsl_stat_t rstat = cmd_ctxt->vmsl.send(cmd_ctxt->msg_ctxt, &req, status);
	if (rstat != VMSL_SUCCESS) {
		fprintf(stderr, "Failed to send command");
		return 1;
	}

	ret = maestro_cmd_disconnect(cmd_ctxt, status);
	if (ret != 0) {
		fprintf(stderr, "Warning error during disconnect");

		return 1;
	}

	return 0;
}