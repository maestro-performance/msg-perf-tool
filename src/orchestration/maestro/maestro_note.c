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
#include "maestro_note.h"

bool maestro_note_payload_prepare(maestro_note_t *note, gru_status_t *status) {
	note->payload = gru_alloc(sizeof(maestro_payload_t), status);
	gru_alloc_check(note->payload, false);

	return true;
}

void maestro_note_payload_cleanup(maestro_note_t *note) {
	if (!note || !note->payload) {
		return;
	}

	if (note->type == MAESTRO_TYPE_RESPONSE) {
		gru_dealloc_string(&note->payload->response.id);
		gru_dealloc_string(&note->payload->response.name);
	}

	if (note->type == MAESTRO_TYPE_REQUEST && note->command == MAESTRO_NOTE_SET) {
		gru_dealloc_string(&note->payload->request.set.value);
	}

	if (note->type == MAESTRO_TYPE_RESPONSE && note->command == MAESTRO_NOTE_STATS) {
		gru_dealloc_string(&note->payload->response.body.stats.role);
		gru_dealloc_string(&note->payload->response.body.stats.roleinfo);
		gru_dealloc_string(&note->payload->response.body.stats.stats.perf.timestamp);
	}

	gru_dealloc((void **) &note->payload);
}

void maestro_note_response_set_id(maestro_note_t *note, const char *id) {
	if (note->type == MAESTRO_TYPE_RESPONSE) {
		note->payload->response.id = strdup(id);
	}

	if (note->type == MAESTRO_TYPE_NOTIFICATION) {
		note->payload->notification.id = strdup(id);
	}

}

void maestro_note_response_set_name(maestro_note_t *note, const char *name) {
	if (note->type == MAESTRO_TYPE_RESPONSE) {
		note->payload->response.name = strdup(name);
	}
	if (note->type == MAESTRO_TYPE_NOTIFICATION) {
		note->payload->notification.name = strdup(name);
	}
}

void maestro_note_notification_set_message(maestro_note_t *note, const char *message) {
	note->payload->notification.body.message = strdup(message);
}

void maestro_note_ping_set_ts(maestro_note_t *note, gru_timestamp_t ts) {
	note->payload->request.ping.sec = ts.tv_sec;
	note->payload->request.ping.usec = ts.tv_usec;
}

void maestro_note_ping_set_elapsed(maestro_note_t *note, uint64_t elapsed) {
	note->payload->response.body.ping.elapsed = elapsed;
}

void maestro_note_set_type(maestro_note_t *note, maestro_request_type type) {
	note->type = type;
}

void maestro_note_set_cmd(maestro_note_t *note, maestro_command_t cmd) {
	note->command = cmd;
}

void maestro_note_set_opt(maestro_note_t *note, int64_t opt, const char *value) {
	maestro_note_payload_prepare(note, NULL);

	note->payload->request.set.opt = opt;
	note->payload->request.set.value = strdup(value);
}

void maestro_note_stats_set_child_count(maestro_note_t *note, uint32_t count) {
	note->payload->response.body.stats.child_count = count;
}

void maestro_note_stats_set_role(maestro_note_t *note, const char *role) {
	note->payload->response.body.stats.role = strdup(role);
}

void maestro_note_stats_set_roleinfo(maestro_note_t *note, const char *roleinfo) {
	note->payload->response.body.stats.roleinfo = strdup(roleinfo);
}

void maestro_note_stats_set_stat_type(maestro_note_t *note, maestro_payload_stat_type_t stat_type) {
	note->payload->response.body.stats.stat_type = stat_type;
}

void maestro_note_stats_set_perf_ts(maestro_note_t *note, const char *ts) {
	note->payload->response.body.stats.stats.perf.timestamp = strdup(ts);
}

void maestro_note_stats_set_perf_count(maestro_note_t *note, uint64_t count) {
	note->payload->response.body.stats.stats.perf.count = count;
}

void maestro_note_stats_set_perf_rate(maestro_note_t *note, double rate) {
	note->payload->response.body.stats.stats.perf.rate = rate;
}

void maestro_note_stats_set_perf_latency(maestro_note_t *note, double latency) {
	note->payload->response.body.stats.stats.perf.latency = latency;
}
