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

bool maestro_note_parse(const void *data, size_t size, maestro_note_t *note, 
	gru_status_t *status)
{
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
		note->payload = gru_alloc(body_len, status);
		if (!note->payload) {
			gru_status_set(status, GRU_FAILURE, "Not enough memory to parse body");

			return false;
		}

		memcpy(note->payload, ((char *) data) + MAESTRO_HEADER_SIZE, body_len);
	}
	else {
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

static void maestro_payload_set_id(maestro_payload_ping_reply_t *pl, const char *id) {
	memcpy(pl->id, id, sizeof(pl->id));
}

static void maestro_payload_set_ts(maestro_payload_ping_reply_t *pl, const char *ts) {
	memcpy(pl->ts, ts, sizeof(pl->ts));
}

void maestro_note_ping_set_id(maestro_note_t *note, const char *id) {
	maestro_payload_set_id(&note->payload->response.ping, id);
}

void maestro_note_ping_set_ts(maestro_note_t *note, const char *ts) {
	maestro_payload_set_ts(&note->payload->response.ping, ts);
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

bool maestro_note_serialize_new(const maestro_note_t *note, msg_content_data_t *out) {
	if (maestro_note_equals(note, MAESTRO_NOTE_OK)) {
		maestro_note_ok_response(out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_PING)) {
		maestro_note_ping_response(out, note->payload->response.ping.id, 
			note->payload->response.ping.ts);
	}
	else {
		maestro_note_protocol_error_response(out);
	}
}


bool maestro_note_serialize(msg_content_data_t *cont, const char *cmd) {
	bool ret = msg_content_data_serialize(cont, "%03s", cmd);
	
	return ret;
}

bool maestro_note_protocol_error_response(msg_content_data_t *cont) {
	bool ret = msg_content_data_serialize(cont, "%03s", 
		maestro_response(MAESTRO_NOTE_PROTOCOL_ERROR));
	
	return ret;
}


bool maestro_note_ok_response(msg_content_data_t *cont) {
	bool ret = msg_content_data_serialize(cont, "%03s", 
		maestro_response(MAESTRO_NOTE_OK));
	
	return ret;
}


bool maestro_note_set_request(msg_content_data_t *cont, const char *opt, const char *val) {
	bool ret = msg_content_data_serialize(cont, "%03s%02s%.250s", 
		maestro_request(MAESTRO_NOTE_SET), " ", opt, val);
	
	return ret;
}


bool maestro_note_ping_request(msg_content_data_t *cont) {
	bool ret = msg_content_data_serialize(cont, "%03s", 
		maestro_request(MAESTRO_NOTE_PING));
	
	return ret;
}



bool maestro_note_ping_response(msg_content_data_t *cont, const char *id, const char *ts) {
	bool ret = msg_content_data_serialize(cont, "%03s%37s%12s", 
		maestro_response(MAESTRO_NOTE_PING), id, ts);
	
	return ret;
}