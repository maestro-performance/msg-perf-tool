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

#include <maestro/maestro_note.h>
#include "maestro_command.h"

static int maestro_cmd_connect(maestro_cmd_ctxt_t *cmd_ctxt,
	gru_uri_t uri,
	gru_status_t *status) {
	logger_t logger = gru_logger_get();

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_SENDER,
		.statistics = MSG_STAT_NONE,
	};

	msg_conn_info_gen_id(&opt.conn_info);

	gru_status_reset(status);
	opt.uri = gru_uri_clone(uri, status);
	if (gru_status_error(status)) {
		logger(GRU_ERROR, "Failed to set connection URL: %s", status->message);
		return -1;
	}

	if (!maestro_cmd_ctxt_start(cmd_ctxt, opt, status)) {
		return 1;
	}

	return 0;
}

static int maestro_cmd_without_payload(maestro_cmd_ctxt_t *cmd_ctxt,
									   const char *path, maestro_command_t cmd, gru_status_t *status) {
	int ret = maestro_cmd_connect(cmd_ctxt, options_get_maestro_uri(), status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, path);

	msg_content_data_t req = {0};
	maestro_easy_request(&req, cmd);

	if (!maestro_cmd_ctxt_send(cmd_ctxt, &req, status)) {
		fprintf(stderr, "Failed to send command to %s: %s\n", path, status->message);
		msg_content_data_release(&req);
		maestro_cmd_ctxt_stop(cmd_ctxt, status);

		return 1;
	}

	msg_content_data_release(&req);
	maestro_cmd_ctxt_stop(cmd_ctxt, status);

	return 0;
}

int maestro_cmd_start_receiver(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_RECEIVER_DAEMONS, MAESTRO_NOTE_START_RECEIVER, status);
}

int maestro_cmd_stop_receiver(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_RECEIVER_DAEMONS, MAESTRO_NOTE_STOP_RECEIVER, status);
}

int maestro_cmd_start_sender(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_SENDER_DAEMONS, MAESTRO_NOTE_START_SENDER, status);
}

int maestro_cmd_stop_sender(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_SENDER_DAEMONS, MAESTRO_NOTE_STOP_SENDER, status);
}

int maestro_cmd_start_inspector(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_BROKER_INSPECTOR_DAEMONS,
									   MAESTRO_NOTE_START_INSPECTOR, status);
}

int maestro_cmd_stop_inspector(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_BROKER_INSPECTOR_DAEMONS,
									   MAESTRO_NOTE_STOP_INSPECTOR, status);
}

int maestro_cmd_start_all(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	maestro_cmd_start_inspector(cmd_ctxt, status);
	maestro_cmd_start_receiver(cmd_ctxt, status);
	return maestro_cmd_start_sender(cmd_ctxt, status);
}

int maestro_cmd_stop_all(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	maestro_cmd_stop_sender(cmd_ctxt, status);
	maestro_cmd_stop_receiver(cmd_ctxt, status);
	return maestro_cmd_stop_inspector(cmd_ctxt, status);
}

static void maestro_cmd_print_responses(maestro_note_t *note) {
	printf("Name: %-45s\tID: %-40s ", note->payload->response.name, note->payload->response.id);

	switch (note->command) {
	case MAESTRO_NOTE_OK: {
		printf("Response: OK\n");
		break;
	}
	case MAESTRO_NOTE_PING: {
		printf("Time: %"PRIu64" ms\n",  note->payload->response.body.ping.elapsed);

		break;
	}
	case MAESTRO_NOTE_STATS: {
		printf("Children: %"PRIu32" Count: %"PRIu64" Rate: %.2f Latency: %.2f\n",
			   note->payload->response.body.stats.child_count,
			   note->payload->response.body.stats.stats.perf.count,
			   note->payload->response.body.stats.stats.perf.rate,
			   note->payload->response.body.stats.stats.perf.latency);
		break;
	}
	case MAESTRO_NOTE_PROTOCOL_ERROR: {
		printf("Error: one of more of the commands did not complete successfully\n");
		break;
	}
	case MAESTRO_NOTE_INTERNAL_ERROR: {
		printf("Error: internal server error\n");
		break;
	}
	default: {
		printf("Error: unhandled response: %d\n", note->command);
		break;
	}
	}
}

static void maestro_cmd_print_notifications(maestro_note_t *note) {
	printf("Notification from: %-45s\tID: %-40s Message: ", note->payload->response.name, note->payload->response.id);

	switch (note->command) {
	case MAESTRO_NOTE_ABNORMAL_DISCONNECT:
	case MAESTRO_NOTE_NOTIFY_FAIL:
	case MAESTRO_NOTE_NOTIFY_SUCCESS: {
		printf("%s\n", note->payload->notification.body.message);
		break;
	}
	default: {
		printf("Error: unhandled notification\n");
		break;
	}
	}
}

static const char *maestro_cmd_get_string(const gru_list_t *strings, uint32_t pos) {
	if (!strings) {
		return NULL;
	}

	const gru_node_t *node = gru_list_get(strings, pos);
	if (!node) {
		return NULL;
	}

	return gru_node_get_data_ptr(char, node);
}

static int maestro_cmd_do_collect(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status)
{
	while (true) {
		char buf[MAESTRO_NOTE_SIZE] = {0};

		ssize_t ret = msgrcv(cmd_ctxt->queue, &buf, sizeof(buf), 0, IPC_NOWAIT);
		if (ret < 0) {
			if (errno == ENOMSG) {
				break;
			}

			int err = errno;
			fprintf(stdout, "Failed to read %"PRIu64" bytes from the local forward queue: %s\n",
				sizeof(buf), strerror(err));

			return 1;

		}

		maestro_note_t note = {0};
		msg_content_data_t msg = {0};

		msg_content_data_copy(&msg, &buf, ret);

		maestro_trace_proto(buf, ret);

		if (!maestro_deserialize_note(&msg, &note, status)) {
			fprintf(stderr, "Unknown protocol data\n");
		} else {
			if (note.type == MAESTRO_TYPE_RESPONSE) {
				maestro_cmd_print_responses(&note);
			}
			else {
				maestro_cmd_print_notifications(&note);
			}
		}

		maestro_note_payload_cleanup(&note);
		msg_content_data_release(&msg);

	}

	return 0;
}

int maestro_cmd_collect(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status)
{
	const char *count_str = maestro_cmd_get_string(strings, 1);
	const char *interval_str = maestro_cmd_get_string(strings, 2);

	if (count_str && interval_str) {
		uint32_t count = atoi(count_str);
		uint32_t interval = atoi(interval_str);

		for (int i = 0; i < count; i++) {
			int ret = maestro_cmd_do_collect(cmd_ctxt, strings, status);
			if (ret != 0) {
				return ret;
			}
			sleep(interval);
		}

		return 0;
	}

	return maestro_cmd_do_collect(cmd_ctxt, strings, status);

}

int maestro_cmd_flush(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_ALL_DAEMONS, MAESTRO_NOTE_FLUSH, status);
}

static int maestro_cmd_set_opt_by_name(msg_content_data_t *data,
	const char *opt,
	const char *val,
	gru_status_t *status) {
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
	} else if (strcmp(opt, "log-level") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_LOG_LEVEL, val);
	} else if (strcmp(opt, "parallel-count") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT, val);
	} else if (strcmp(opt, "message-size") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE, val);
	} else if (strcmp(opt, "throttle") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_THROTTLE, val);
	} else if (strcmp(opt, "rate") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_SET_RATE, val);
	} else if (strcmp(opt, "fcl") == 0) {
		maestro_note_set_opt(&note, MAESTRO_NOTE_OPT_FCL, val);
	} else {
		return -1;
	}

	maestro_serialize_note(&note, data);
	maestro_note_payload_cleanup(&note);
	return 0;
}

int maestro_cmd_set_opt(maestro_cmd_ctxt_t *cmd_ctxt,
	gru_list_t *strings,
	gru_status_t *status) {

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

	int ret = maestro_cmd_connect(cmd_ctxt, options_get_maestro_uri(), status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, MAESTRO_ALL_DAEMONS);

	msg_content_data_t req = {0};
	if (maestro_cmd_set_opt_by_name(&req, opt, val, status) == -1) {
		gru_status_set(status, GRU_FAILURE, "Invalid option: %s", opt);
		maestro_cmd_ctxt_stop(cmd_ctxt, status);

		return 1;
	}

	if (!maestro_cmd_ctxt_send(cmd_ctxt, &req, status)) {
		fprintf(stderr, "Failed to send command: %s\n", status->message);
		msg_content_data_release(&req);
		maestro_cmd_ctxt_stop(cmd_ctxt, status);

		return 1;
	}

	msg_content_data_release(&req);
	maestro_cmd_ctxt_stop(cmd_ctxt, status);

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
	maestro_note_ping_set_ts(&note, ts);
	maestro_serialize_note(&note, out);
	maestro_note_payload_cleanup(&note);
}

int maestro_cmd_ping(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status) {
	int ret = maestro_cmd_connect(cmd_ctxt, options_get_maestro_uri(), status);
	if (ret != 0) {
		return ret;
	}

	gru_uri_set_path(&cmd_ctxt->msg_ctxt->msg_opts.uri, MAESTRO_ALL_DAEMONS);

	msg_content_data_t req = {0};
	maestro_cmd_request_ping(&req, status);
	if (!gru_status_success(status)) {
		return 1;
	}

	if (!maestro_cmd_ctxt_send(cmd_ctxt, &req, status)) {
		fprintf(stderr, "Failed to send command: %s\n", status->message);
		msg_content_data_release(&req);
		maestro_cmd_ctxt_stop(cmd_ctxt, status);

		return 1;
	}

	msg_content_data_release(&req);
	maestro_cmd_ctxt_stop(cmd_ctxt, status);

	return 0;
}

int maestro_cmd_stats(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status)
{
	const char *count_str = maestro_cmd_get_string(strings, 1);
	const char *interval_str = maestro_cmd_get_string(strings, 2);

	if (count_str && interval_str) {
		uint32_t count = atoi(count_str);
		uint32_t interval = atoi(interval_str);

		for (int i = 0; i < count; i++) {
			int ret = 0;

			ret = maestro_cmd_without_payload(cmd_ctxt, MAESTRO_ALL_DAEMONS, MAESTRO_NOTE_STATS, status);
			if (ret != 0) {
				return ret;
			}

			ret = maestro_cmd_do_collect(cmd_ctxt, NULL, status);
			if (ret != 0) {
				return ret;
			}

			sleep(interval);
		}

		return 0;
	}

	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_ALL_DAEMONS, MAESTRO_NOTE_STATS, status);

}

int maestro_cmd_halt(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status)
{
	return maestro_cmd_without_payload(cmd_ctxt, MAESTRO_ALL_DAEMONS, MAESTRO_NOTE_HALT, status);
}