/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "maestro_serialize.h"

static bool maestro_serialize_header_only(const maestro_note_t *note,
	msg_content_data_t *out) {
	bool ret = msg_content_data_serialize(
		out, "%c%.*s", note->type, sizeof(note->command), note->command);

	return ret;
}

static bool maestro_note_set_request(const maestro_note_t *note,
	msg_content_data_t *cont) {
	bool ret = msg_content_data_serialize(cont,
		"%c%.*s%.*s%.*s",
		note->type,
		sizeof(note->command),
		note->command,
		sizeof(note->payload->request.set.opt),
		note->payload->request.set.opt,
		sizeof(note->payload->request.set.value),
		note->payload->request.set.value);

	return ret;
}

bool maestro_serialize_note(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false;

	if (maestro_note_equals(note, MAESTRO_NOTE_OK)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_PROTOCOL_ERROR)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_INTERNAL_ERROR)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_INTERNAL_ERROR)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_START_RECEIVER)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_STOP_RECEIVER)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_START_SENDER)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_STOP_SENDER)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_START_INSPECTOR)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_STOP_INSPECTOR)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_HALT)) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_SET) &&
		note->type == MAESTRO_TYPE_REQUEST) {
		ret = maestro_note_set_request(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_FLUSH) &&
		note->type == MAESTRO_TYPE_REQUEST) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_PING) &&
		note->type == MAESTRO_TYPE_RESPONSE) {
		ret = msg_content_data_serialize(out,
			"%c%.*s%.*s%.*s",
			note->type,
			sizeof(note->command),
			note->command,
			sizeof(note->payload->response.ping.id),
			note->payload->response.ping.id,
			sizeof(note->payload->response.ping.elapsed),
			note->payload->response.ping.elapsed);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_PING) &&
		note->type == MAESTRO_TYPE_REQUEST) {
		ret = msg_content_data_serialize(out,
			"%c%.*s%.*s",
			note->type,
			sizeof(note->command),
			note->command,
			sizeof(note->payload->request.ping.ts),
			note->payload->request.ping.ts);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_STATS) &&
		note->type == MAESTRO_TYPE_REQUEST) {
		ret = maestro_serialize_header_only(note, out);
	} else if (maestro_note_equals(note, MAESTRO_NOTE_STATS) &&
		note->type == MAESTRO_TYPE_RESPONSE) {
		// '10527ef92d6-96ad-4ff6-ab3a-d547395f2c0b3receiverperfR1493977868.543546971993841.20165.00'
		ret = msg_content_data_serialize(out,
			"%c%-*s%-*s%-*s%-*s%-*s%c%-*s%-*s%-*s%-*s",
			note->type,
			(int) sizeof(note->command),
			note->command,
			(int) sizeof(note->payload->response.stats.id),
			note->payload->response.stats.id,
			(int) sizeof(note->payload->response.stats.child_count),
			note->payload->response.stats.child_count,
			(int) sizeof(note->payload->response.stats.role),
			note->payload->response.stats.role,
			(int) sizeof(note->payload->response.stats.roleinfo),
			note->payload->response.stats.roleinfo,
			note->payload->response.stats.stat_type,
			(int) sizeof(note->payload->response.stats.stats.perf.timestamp),
			note->payload->response.stats.stats.perf.timestamp,
			(int) sizeof(note->payload->response.stats.stats.perf.count),
			note->payload->response.stats.stats.perf.count,
			(int) sizeof(note->payload->response.stats.stats.perf.rate),
			note->payload->response.stats.stats.perf.rate,
			(int) sizeof(note->payload->response.stats.stats.perf.latency),
			note->payload->response.stats.stats.perf.latency);
		// I am doing something wrong, because vasprintf keeps writing more data than
		// requeste in the format string. Therefore, force the out content to only have
		// the adequate size
		out->size =
			1 + (int) sizeof(note->command) + sizeof(maestro_payload_stats_reply_t);
	} else {
		ret = msg_content_data_serialize(out,
			"%c%.*s",
			note->type,
			sizeof(note->command),
			MAESTRO_NOTE_PROTOCOL_ERROR);
	}

	return ret;
}