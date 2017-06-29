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

static bool maestro_serialize_header_only(const maestro_note_t *note,
	msg_content_data_t *out) {

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_char(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

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

	msgpack_pack_char(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);
	msgpack_pack_int64(&pk, note->payload->request.set.opt);

	msgpack_pack_str(&pk, strlen(note->payload->request.set.value));
	msgpack_pack_str_body(&pk, note->payload->request.set.value,
		strlen(note->payload->request.set.value));

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

static bool maestro_serialize_ping_request(const maestro_note_t *note, msg_content_data_t *out) {
	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_char(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

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

	msgpack_pack_char(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	msgpack_pack_str(&pk, strlen(note->payload->response.ping.id));
	msgpack_pack_str_body(
		&pk, note->payload->response.ping.id, strlen(note->payload->response.ping.id));

  	msgpack_pack_str(&pk, strlen(note->payload->response.ping.name));
  	msgpack_pack_str_body(
	  &pk, note->payload->response.ping.name, strlen(note->payload->response.ping.name));

  	msgpack_pack_uint64(&pk, note->payload->response.ping.elapsed);
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

	msgpack_pack_char(&pk, note->type);
	msgpack_pack_int64(&pk, note->command);

	msgpack_pack_str(&pk, strlen(note->payload->response.stats.id));
	msgpack_pack_str_body(
		&pk, note->payload->response.stats.id, strlen(note->payload->response.stats.id));

	msgpack_pack_str(&pk, strlen(note->payload->response.stats.name));
	msgpack_pack_str_body(
		&pk, note->payload->response.stats.name, strlen(note->payload->response.stats.name));

	msgpack_pack_uint32(&pk, note->payload->response.stats.child_count);

	msgpack_pack_str(&pk, strlen(note->payload->response.stats.role));
	msgpack_pack_str_body(
		&pk, note->payload->response.stats.role, strlen(note->payload->response.stats.role));

	msgpack_pack_str(&pk, strlen(note->payload->response.stats.roleinfo));
	msgpack_pack_str_body(
		&pk, note->payload->response.stats.roleinfo, strlen(note->payload->response.stats.roleinfo));

	msgpack_pack_uint8(&pk, note->payload->response.stats.stat_type);

	msgpack_pack_str(&pk, strlen(note->payload->response.stats.stats.perf.timestamp));
	msgpack_pack_str_body(
		&pk, note->payload->response.stats.stats.perf.timestamp, strlen(note->payload->response.stats.stats.perf.timestamp));

	msgpack_pack_uint64(&pk, note->payload->response.stats.stats.perf.count);
	msgpack_pack_double(&pk, note->payload->response.stats.stats.perf.rate);
	msgpack_pack_double(&pk, note->payload->response.stats.stats.perf.latency);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

bool maestro_serialize_note(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false;

	switch (note->command) {
		case MAESTRO_NOTE_OK:
		case MAESTRO_NOTE_PROTOCOL_ERROR:
		case MAESTRO_NOTE_INTERNAL_ERROR:
		case MAESTRO_NOTE_START_RECEIVER:
		case MAESTRO_NOTE_STOP_RECEIVER:
		case MAESTRO_NOTE_START_SENDER:
		case MAESTRO_NOTE_STOP_SENDER:
		case MAESTRO_NOTE_START_INSPECTOR:
		case MAESTRO_NOTE_STOP_INSPECTOR:
		case MAESTRO_NOTE_FLUSH:
		case MAESTRO_NOTE_HALT: {
			ret = maestro_serialize_header_only(note, out);
			break;
		}
		case MAESTRO_NOTE_SET: {
			ret = maestro_note_set_request(note, out);
			break;
		}
		case MAESTRO_NOTE_PING: {
			if (note->type == MAESTRO_TYPE_REQUEST) {
				ret = maestro_serialize_ping_request(note, out);
			}
		  	else {
				ret = maestro_serialize_ping_response(note, out);
			}

			break;
		}
		case MAESTRO_NOTE_STATS: {
			if (note->type == MAESTRO_TYPE_REQUEST) {
			  ret = maestro_serialize_header_only(note, out);
		  	}
		  	else {
				ret = maestro_serialize_stats_response(note, out);
		  	}

		  	break;
		}

	}



	// 	note->type == MAESTRO_TYPE_RESPONSE) {
	// 	// '10527ef92d6-96ad-4ff6-ab3a-d547395f2c0b3receiverperfR1493977868.543546971993841.20165.00'
	// 	ret = msg_content_data_serialize(out,
	// 		"%c%-*s%-*s%-*s%-*s%-*s%-*s%c%-*s%-*s%-*s%-*s",
	// 		note->type,
	// 		(int) sizeof(note->command),
	// 		note->command,
	// 		(int) sizeof(note->payload->response.stats.id),
	// 		note->payload->response.stats.id,
	// 		(int) sizeof(note->payload->response.stats.name),
	// 		note->payload->response.stats.name,
	// 		(int) sizeof(note->payload->response.stats.child_count),
	// 		note->payload->response.stats.child_count,
	// 		(int) sizeof(note->payload->response.stats.role),
	// 		note->payload->response.stats.role,
	// 		(int) sizeof(note->payload->response.stats.roleinfo),
	// 		note->payload->response.stats.roleinfo,
	// 		note->payload->response.stats.stat_type,
	// 		(int) sizeof(note->payload->response.stats.stats.perf.timestamp),
	// 		note->payload->response.stats.stats.perf.timestamp,
	// 		(int) sizeof(note->payload->response.stats.stats.perf.count),
	// 		note->payload->response.stats.stats.perf.count,
	// 		(int) sizeof(note->payload->response.stats.stats.perf.rate),
	// 		note->payload->response.stats.stats.perf.rate,
	// 		(int) sizeof(note->payload->response.stats.stats.perf.latency),
	// 		note->payload->response.stats.stats.perf.latency);
	// 	// I am doing something wrong, because vasprintf keeps writing more data than
	// 	// requeste in the format string. Therefore, force the out content to only have
	// 	// the adequate size
	// 	out->size =
	// 		1 + (int) sizeof(note->command) + sizeof(maestro_payload_stats_reply_t);
	// } else {
	// 	ret = msg_content_data_serialize(out,
	// 		"%c%.*s",
	// 		note->type,
	// 		sizeof(note->command),
	// 		MAESTRO_NOTE_PROTOCOL_ERROR);
	// }

	return ret;
}

