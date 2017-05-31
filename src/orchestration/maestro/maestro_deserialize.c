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
#include "maestro_deserialize.h"

static bool maestro_deserialize_note_assign(const msgpack_object obj,
	void *out,
	gru_status_t *status) {
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
			snprintf((char *) out,
				obj.via.str.size + 1,
				"%.*s",
				obj.via.str.size,
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

static bool maestro_deserialize_note_set_request(const msg_content_data_t *in,
	maestro_note_t *note,
	msgpack_unpacked *msg,
	size_t *offset,
	gru_status_t *status) {
	if (!maestro_note_payload_prepare(note, status)) {
		return false;
	}

	// Set command
	msgpack_unpack_return ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack set command: invalid and/or missing command");

		return false;
	}

	if (!maestro_deserialize_note_assign(
			msg->data, &note->payload->request.set.opt, status)) {
		return false;
	}

	// Set value
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack set value: invalid and/or missing value");

		return false;
	}

	note->payload->request.set.value = gru_alloc(msg->data.via.str.size + 1, status);
	gru_alloc_check(note->payload->request.set.value, false);

	if (!maestro_deserialize_note_assign(
			msg->data, note->payload->request.set.value, status)) {
		return false;
	}

	return true;
}

static bool maestro_deserialize_note_ping_request(const msg_content_data_t *in,
	maestro_note_t *note,
	msgpack_unpacked *msg,
	size_t *offset,
	gru_status_t *status) {
	if (!maestro_note_payload_prepare(note, status)) {
		return false;
	}

	// Ping timestamp
	msgpack_unpack_return ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack ping timestamp: invalid and/or missing timestamp");

		return false;
	}

	if (!maestro_deserialize_note_assign(
			msg->data, &note->payload->request.ping.ts, status)) {
		return false;
	}

	return true;
}

bool maestro_deserialize_note(const msg_content_data_t *in,
	maestro_note_t *note,
	gru_status_t *status) {
	size_t offset = 0;
	msgpack_unpacked msg;

	msgpack_unpacked_init(&msg);

	msgpack_unpack_return ret;

	// Deserialize type
	ret = msgpack_unpack_next(&msg, in->data, in->size, &offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack protocol data: invalid and/or missing note type");

		return false;
	}

	if (msg.data.type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to unpack protocol data: invalid note type");
	} else {
		if (!maestro_deserialize_note_assign(msg.data, &note->type, status)) {
			goto err_exit;
		}
	}

	ret = msgpack_unpack_next(&msg, in->data, in->size, &offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack protocol data: invalid and/or missing command");

		return false;
	} else {
		if (!maestro_deserialize_note_assign(msg.data, &note->command, status)) {
			goto err_exit;
		}
	}

	bool pl_ret = true;

	switch (note->command) {
		case MAESTRO_NOTE_SET: {
			if (note->type == MAESTRO_TYPE_REQUEST) {
				pl_ret =
					maestro_deserialize_note_set_request(in, note, &msg, &offset, status);
			}

			break;
		}
		case MAESTRO_NOTE_PING: {
			if (note->type == MAESTRO_TYPE_REQUEST) {
				pl_ret = maestro_deserialize_note_ping_request(
					in, note, &msg, &offset, status);
			}

			break;
		}
	}

	if (!pl_ret) {
		goto err_exit;
	}

	msgpack_unpacked_destroy(&msg);
	return true;

err_exit:
	msgpack_unpacked_destroy(&msg);
	return false;
}