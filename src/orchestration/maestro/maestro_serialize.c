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

static void maestro_serialize_response_header(const maestro_note_t *note, msgpack_packer *pk) {
	msgpack_pack_char(pk, note->type);
	msgpack_pack_int64(pk, note->command);

	msgpack_pack_str(pk, strlen(note->payload->response.id));
	msgpack_pack_str_body(
		pk, note->payload->response.id, strlen(note->payload->response.id));

	msgpack_pack_str(pk, strlen(note->payload->response.name));
	msgpack_pack_str_body(
		pk, note->payload->response.name, strlen(note->payload->response.name));
}

static bool maestro_serialize_header_only(const maestro_note_t *note,
	msg_content_data_t *out) {

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_char(&pk, note->type);
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

	maestro_serialize_response_header(note, &pk);

	msgpack_pack_uint32(&pk, note->payload->response.body.stats.child_count);

	msgpack_pack_str(&pk, strlen(note->payload->response.body.stats.role));
	msgpack_pack_str_body(
		&pk, note->payload->response.body.stats.role, strlen(note->payload->response.body.stats.role));

	msgpack_pack_str(&pk, strlen(note->payload->response.body.stats.roleinfo));
	msgpack_pack_str_body(
		&pk, note->payload->response.body.stats.roleinfo, strlen(note->payload->response.body.stats.roleinfo));

	msgpack_pack_uint8(&pk, note->payload->response.body.stats.stat_type);

	msgpack_pack_str(&pk, strlen(note->payload->response.body.stats.stats.perf.timestamp));
	msgpack_pack_str_body(
		&pk, note->payload->response.body.stats.stats.perf.timestamp,
		strlen(note->payload->response.body.stats.stats.perf.timestamp));

	msgpack_pack_uint64(&pk, note->payload->response.body.stats.stats.perf.count);
	msgpack_pack_double(&pk, note->payload->response.body.stats.stats.perf.rate);
	msgpack_pack_double(&pk, note->payload->response.body.stats.stats.perf.latency);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
}

bool maestro_serialize_note(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false;

	switch (note->command) {
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
		default: {
			ret = maestro_serialize_header_only(note, out);
			break;
		}
	}

	return ret;
}

