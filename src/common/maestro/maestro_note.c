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

	
	if (size < MAESTRO_HEADER_SIZE) {
		gru_status_set(status, GRU_FAILURE, "Not enough data to parse");

		return false;
	}

	const char *str = (const char *) data;
	if (!isalnum(str[0])) {
		gru_status_set(status, GRU_FAILURE, "Invalid note type at position 0");

		return false;
	}
	note->type = str[0];

	if (!isalnum(str[1]) || !isalnum(str[2])) {
		gru_status_set(status, GRU_FAILURE, "Invalid note command");

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
}


bool maestro_note_serialize(msg_content_data_t *cont, const char *cmd) {
	bool ret = msg_content_data_serialize(cont, "%03s", cmd);
	
	return ret;
}