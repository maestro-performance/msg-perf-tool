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
#include "maestro_easy.h"

void maestro_easy_request(msg_content_data_t *out, const char *cmd) {
	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_REQUEST);
	maestro_note_set_cmd(&note, cmd);
	
	maestro_serialize_note(&note, out);
}

void maestro_easy_response(msg_content_data_t *out, const char *cmd) {

	maestro_note_t note = {0};

	maestro_note_set_type(&note, MAESTRO_TYPE_RESPONSE);
	maestro_note_set_cmd(&note, cmd);
	
	maestro_serialize_note(&note, out);
}