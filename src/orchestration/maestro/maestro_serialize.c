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
#include "maestro_note.h"

static inline void maestro_serialize_str_field(msgpack_packer *pk, const char *str) {
	size_t size = strlen(str);

	msgpack_pack_str(pk, size);
	msgpack_pack_str_body(pk, str, size);
}


static void maestro_serialize_response_header(const maestro_note_t *note, msgpack_packer *pk) {
	maestro_serialize_str_field(pk, note->payload->response.id);
	maestro_serialize_str_field(pk, note->payload->response.name);
}

static void maestro_serialize_notification_header(const maestro_note_t *note, msgpack_packer *pk) {
	maestro_serialize_str_field(pk, note->payload->response.id);
	maestro_serialize_str_field(pk, note->payload->notification.name);
}

static bool maestro_serialize_empty_exchange(const maestro_note_t *note,
											 msg_content_data_t *out) {

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_int8(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);


	if (note->type == MAESTRO_TYPE_RESPONSE) {
		maestro_serialize_response_header(note, &pk);
	}

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}


static bool maestro_note_set_request(const maestro_note_t *note,
	msg_content_data_t *out) {

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_int8(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	if (!note->payload) {
		logger_t logger = gru_logger_get();

		logger(GRU_ERROR, "Invalid set request: null payload");
		return false;
	}

	msgpack_pack_int64(&pk, note->payload->request.set.opt);

	maestro_serialize_str_field(&pk, note->payload->request.set.value);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

static bool maestro_serialize_ping_request(const maestro_note_t *note, msg_content_data_t *out) {
	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_int8(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	if (!note->payload) {
		logger_t logger = gru_logger_get();

		logger(GRU_ERROR, "Invalid ping request: null payload");
		return false;
	}

	msgpack_pack_uint64(&pk, note->payload->request.ping.sec);
	msgpack_pack_uint64(&pk, note->payload->request.ping.usec);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}



static bool maestro_serialize_ping_response(const maestro_note_t *note,
	msg_content_data_t *out) {
	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_int8(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	maestro_serialize_response_header(note, &pk);

  	msgpack_pack_uint64(&pk, note->payload->response.body.ping.elapsed);
	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

static bool maestro_serialize_stats_response(const maestro_note_t *note,
											msg_content_data_t *out) {
	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_int8(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	maestro_serialize_response_header(note, &pk);

	msgpack_pack_uint32(&pk, note->payload->response.body.stats.child_count);

	maestro_serialize_str_field(&pk, note->payload->response.body.stats.role);
	maestro_serialize_str_field(&pk, note->payload->response.body.stats.roleinfo);

	msgpack_pack_uint8(&pk, note->payload->response.body.stats.stat_type);

	maestro_serialize_str_field(&pk,
								note->payload->response.body.stats.stats.perf.timestamp);

	msgpack_pack_uint64(&pk, note->payload->response.body.stats.stats.perf.count);
	msgpack_pack_double(&pk, note->payload->response.body.stats.stats.perf.rate);
	msgpack_pack_double(&pk, note->payload->response.body.stats.stats.perf.latency);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

static bool maestro_serialize_notification_exchange(const maestro_note_t *note,
											 msg_content_data_t *out) {

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_int8(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	maestro_serialize_notification_header(note, &pk);

	maestro_serialize_str_field(&pk, note->payload->notification.body.message);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

static bool maestro_serialize_request(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false;

	switch (note->command) {
		case MAESTRO_NOTE_SET: {
			ret = maestro_note_set_request(note, out);
			break;
		}
		case MAESTRO_NOTE_PING: {
			ret = maestro_serialize_ping_request(note, out);

			break;
		}
		case MAESTRO_NOTE_STATS: {
			ret = maestro_serialize_empty_exchange(note, out);

			break;
		}
		default: {
			ret = maestro_serialize_empty_exchange(note, out);

			break;
		}
	}

	return ret;
}

static bool maestro_serialize_response(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false;

	switch (note->command) {
		case MAESTRO_NOTE_PING: {
			ret = maestro_serialize_ping_response(note, out);

			break;
		}
		case MAESTRO_NOTE_STATS: {
			ret = maestro_serialize_stats_response(note, out);

			break;
		}
		default: {
			ret = maestro_serialize_empty_exchange(note, out);
			break;
		}
	}

	return ret;
}

static bool maestro_serialize_notification(const maestro_note_t *note, msg_content_data_t *out) {
	return maestro_serialize_notification_exchange(note, out);
}

bool maestro_serialize_note(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false;

	switch (note->type) {
		case MAESTRO_TYPE_REQUEST: {
			ret = maestro_serialize_request(note, out);

			break;
		}
		case MAESTRO_TYPE_RESPONSE: {
			ret = maestro_serialize_response(note, out);

			break;
		}
		case MAESTRO_TYPE_NOTIFICATION: {
			ret = maestro_serialize_notification(note, out);

			break;
		}
	}

	return ret;
}

