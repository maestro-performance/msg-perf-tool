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
	note->payload = gru_alloc(MAESTRO_NOTE_PAYLOAD_MAX_LENGTH, status);
	gru_alloc_check(note->payload, false);

	return true;
}

void maestro_note_payload_cleanup(maestro_note_t *note) {
	if (!note || !note->payload) {
		return;
	}

	gru_dealloc((void **) &note->payload);
}

bool maestro_note_parse(const void *data,
	size_t size,
	maestro_note_t *note,
	gru_status_t *status) {
	if (!data) {
		gru_status_set(status, GRU_FAILURE, "Invalid data");

		return false;
	}

	if (!note) {
		gru_status_set(status, GRU_FAILURE, "Missing output structure");

		return false;
	}

	const char *str = (const char *) data;
	if (!isxdigit(str[0])) {
		gru_status_set(status, GRU_FAILURE, "Invalid note type at position 0");

		return false;
	}
	note->type = str[0];

	if (!isxdigit(str[1]) || !isxdigit(str[2])) {
		gru_status_set(status, GRU_FAILURE, "Invalid note command");

		return false;
	}

	if (size < MAESTRO_HEADER_SIZE) {
		gru_status_set(status, GRU_FAILURE, "Not enough data to parse response data");

		return false;
	}

	bzero(note->command, sizeof(note->command));
	note->command[0] = str[1];
	note->command[1] = str[2];

	size_t body_len = size - MAESTRO_HEADER_SIZE;

	if (body_len > 0) {
		if (!maestro_note_payload_prepare(note, status)) {
			gru_status_set(status, GRU_FAILURE, "Not enough memory to parse body");

			return false;
		}

		memcpy(note->payload, ((char *) data) + MAESTRO_HEADER_SIZE, body_len);
	} else {
		note->payload = NULL;
	}

	return true;
}

bool maestro_note_equals(const maestro_note_t *note, const char *cmd) {
	if (strncmp(note->command, cmd, MAESTRO_NOTE_CMD_LENGTH) == 0) {
		return true;
	}

	return false;
}

static void maestro_payload_set_req_ts(maestro_payload_ping_request_t *pl,
	const char *ts) {
	memcpy(pl->ts, ts, sizeof(pl->ts));
}

void maestro_note_ping_set_id(maestro_note_t *note, const char *id) {
	maestro_set_payload_txt_field(note->payload->response.ping.id, id);
}

void maestro_note_ping_set_name(maestro_note_t *note, const char *name) {
	maestro_set_payload_txt_field(note->payload->response.ping.name, name);
}

void maestro_note_ping_set_ts(maestro_note_t *note, const char *ts) {
	maestro_payload_set_req_ts(&note->payload->request.ping, ts);
}

void maestro_note_ping_set_elapsed(maestro_note_t *note, uint64_t elapsed) {
	maestro_set_payload_uint64_field(note->payload->response.ping.elapsed, elapsed);
}

void maestro_note_set_type(maestro_note_t *note, const char type) {
	note->type = type;
}

void maestro_note_set_cmd(maestro_note_t *note, const char *cmd) {
	if (cmd == NULL) {
		strncpy(note->command, MAESTRO_NOTE_PROTOCOL_ERROR, sizeof(note->command));

		return;
	}

	if (strlen(cmd) != MAESTRO_NOTE_CMD_LENGTH) {
		strncpy(note->command, MAESTRO_NOTE_PROTOCOL_ERROR, sizeof(note->command));

		return;
	}

	bzero(note->command, sizeof(note->command));
	note->command[0] = cmd[0];
	note->command[1] = cmd[1];
}

static void maestro_payload_set_opt(maestro_payload_set_t *pl, const char *opt) {
	memcpy(pl->opt, opt, sizeof(pl->opt));
}

static void maestro_payload_set_value(maestro_payload_set_t *pl, const char *value) {
	memcpy(pl->value, value, sizeof(pl->value));
}

void maestro_note_set_opt(maestro_note_t *note, const char *opt, const char *value) {
	maestro_payload_set_opt(&note->payload->request.set, opt);
	maestro_payload_set_value(&note->payload->request.set, value);
}

void maestro_note_stats_set_id(maestro_note_t *note, const char *id) {
	maestro_set_payload_txt_field(note->payload->response.stats.id, id);
}

void maestro_note_stats_set_child_count(maestro_note_t *note, uint32_t count) {
	maestro_set_payload_uint32_field(note->payload->response.stats.child_count, count);
}

void maestro_note_stats_set_role(maestro_note_t *note, const char *role) {
	maestro_set_payload_txt_field(note->payload->response.stats.role, role);
}

void maestro_note_stats_set_roleinfo(maestro_note_t *note, const char *roleinfo) {
	maestro_set_payload_txt_field(note->payload->response.stats.roleinfo, roleinfo);
}

void maestro_note_stats_set_stat_type(maestro_note_t *note, const char stat_type) {
	note->payload->response.stats.stat_type = stat_type;
}

void maestro_note_stats_set_perf_ts(maestro_note_t *note, const char *ts) {
	maestro_set_payload_txt_field(note->payload->response.stats.stats.perf.timestamp, ts);
}

void maestro_note_stats_set_perf_count(maestro_note_t *note, uint64_t count) {
	maestro_set_payload_uint64_field(
		note->payload->response.stats.stats.perf.count, count);
}

void maestro_note_stats_set_perf_rate(maestro_note_t *note, double rate) {
	maestro_set_payload_double_field(note->payload->response.stats.stats.perf.rate, rate);
}

void maestro_note_stats_set_perf_latency(maestro_note_t *note, double latency) {
	maestro_set_payload_double_field(
		note->payload->response.stats.stats.perf.latency, latency);
}
