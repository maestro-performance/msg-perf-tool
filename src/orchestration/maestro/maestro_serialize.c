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


void print(char const* buf, unsigned int len)
{
    size_t i = 0;
    for(; i < len ; ++i)
        printf("%02x ", 0xff & buf[i]);
    printf("\n");
}

static bool maestro_serialize_header_only(const maestro_note_t *note,
	msg_content_data_t *out) {

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	msgpack_pack_char(&pk, note->type);


	msgpack_pack_int64(&pk, note->command);

	print(sbuf.data, sbuf.size);

	msg_content_data_copy(out, sbuf.data, sbuf.size);

	msgpack_sbuffer_destroy(&sbuf);

	return true;
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
			"%c%-*s%-*s%-*s%-*s",
			note->type,
			sizeof(note->command),
			note->command,
			sizeof(note->payload->response.ping.id),
			note->payload->response.ping.id,
			sizeof(note->payload->response.ping.name),
			note->payload->response.ping.name,
			sizeof(note->payload->response.ping.elapsed),
			note->payload->response.ping.elapsed);
		out->size =
			1 + (int) sizeof(note->command) + sizeof(maestro_payload_ping_reply_t);
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
			"%c%-*s%-*s%-*s%-*s%-*s%-*s%c%-*s%-*s%-*s%-*s",
			note->type,
			(int) sizeof(note->command),
			note->command,
			(int) sizeof(note->payload->response.stats.id),
			note->payload->response.stats.id,
			(int) sizeof(note->payload->response.stats.name),
			note->payload->response.stats.name,
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

static bool maestro_deserialize_note_set(const msgpack_object obj, void *out, gru_status_t *status) {
	switch (obj.type) {
		case MSGPACK_OBJECT_BOOLEAN: {
			(*(bool *) out) = obj.via.boolean;
			break;
		}
		case MSGPACK_OBJECT_POSITIVE_INTEGER: {
			(*(int64_t *) out) = obj.via.i64;
			break;
		}
		case MSGPACK_OBJECT_NEGATIVE_INTEGER: {
			(*(uint64_t *) out) = obj.via.u64;
			break;
		}
		case MSGPACK_OBJECT_FLOAT: {
			(*(double *) out) = obj.via.f64;
			break;
		}
		case MSGPACK_OBJECT_STR: {
			snprintf((char *) out, obj.via.str.size, "%.*s", obj.via.str.size,
				obj.via.str.ptr);
			break;
		}
		case MSGPACK_OBJECT_NIL:
		case MSGPACK_OBJECT_ARRAY:
		case MSGPACK_OBJECT_MAP:
		case MSGPACK_OBJECT_BIN:
		case MSGPACK_OBJECT_EXT:
		default: {
			gru_status_set(status, GRU_FAILURE, "Unsupported type: %d", obj.type);
			return false;
		}
	}

	return true;
}

bool maestro_deserialize_note(const msg_content_data_t *in, maestro_note_t *note,
	gru_status_t *status)
{
	size_t offset = 0;
	msgpack_unpacked msg;

	msgpack_unpacked_init(&msg);

	msgpack_unpack_return ret;

	// Deserialize type
	ret = msgpack_unpack_next(&msg, in->data, in->size, &offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status, GRU_FAILURE, "Unable to unpack protocol data: invalid and/or missing note type");

		return false;
	}

	if (msg.data.type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
		gru_status_set(status, GRU_FAILURE,  "Unable to unpack protocol data: invalid note type");
	}
	else {
		if (!maestro_deserialize_note_set(msg.data, &note->type, status)) {
			goto err_exit;
		}
	}


	ret = msgpack_unpack_next(&msg, in->data, in->size, &offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status, GRU_FAILURE, "Unable to unpack protocol data: invalid and/or missing command");

		return false;
	}
	else {
		if (!maestro_deserialize_note_set(msg.data, &note->command, status)) {
			goto err_exit;
		}
	}

	msgpack_unpacked_destroy(&msg);
	return true;

	err_exit:
	msgpack_unpacked_destroy(&msg);
	return false;
}