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


static bool maestro_serialize_header_only(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = msg_content_data_serialize(out, "%c%.*s", note->type, 
				sizeof(note->command), note->command);
	
	return ret;
}








// static bool maestro_note_ping_request(msg_content_data_t *cont) {
// 	bool ret = msg_content_data_serialize(cont, "%03s", 
// 		maestro_request(MAESTRO_NOTE_PING));
	
// 	return ret;
// }



// static bool maestro_note_ping_response(msg_content_data_t *cont, const char *id, const char *ts) {
// 	bool ret = msg_content_data_serialize(cont, "%03s%37s%12s", 
// 		maestro_response(MAESTRO_NOTE_PING), id, ts);
	
// 	return ret;
// }

static bool maestro_note_set_request(const maestro_note_t *note, msg_content_data_t *cont) {
	bool ret = msg_content_data_serialize(cont, "%c%.*s%.*s%.*s", 
		note->type, 
		sizeof(note->command), note->command, 
		sizeof(note->payload->request.set.opt), note->payload->request.set.opt, 
		sizeof(note->payload->request.set.value), note->payload->request.set.value);
	
 	return ret;
}


bool maestro_serialize_note(const maestro_note_t *note, msg_content_data_t *out) {
	bool ret = false; 

	if (maestro_note_equals(note, MAESTRO_NOTE_OK)) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_PROTOCOL_ERROR)) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_INTERNAL_ERROR)) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_INTERNAL_ERROR)) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_START)) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_STOP)) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_SET) && note->type == MAESTRO_TYPE_REQUEST) {
		ret = maestro_note_set_request(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_FLUSH) && note->type == MAESTRO_TYPE_REQUEST) {
		ret = maestro_serialize_header_only(note, out);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_PING) && note->type == MAESTRO_TYPE_RESPONSE) {
			ret = msg_content_data_serialize(out, "%c%.*s%.*s%.*s", 
				note->type, 
				sizeof(note->command), note->command, 
				sizeof(note->payload->response.ping.id), note->payload->response.ping.id, 
				sizeof(note->payload->response.ping.elapsed), note->payload->response.ping.elapsed);
	}
	else if (maestro_note_equals(note, MAESTRO_NOTE_PING) && note->type == MAESTRO_TYPE_REQUEST) {
			ret = msg_content_data_serialize(out, "%c%.*s%.*s", 
				note->type, 
				sizeof(note->command), note->command, 
				sizeof(note->payload->request.ping.ts), note->payload->request.ping.ts);
	}
	else {
		ret = msg_content_data_serialize(out, "%c%.*s", note->type, 
				sizeof(note->command), MAESTRO_NOTE_PROTOCOL_ERROR);
	}

	return ret;
}