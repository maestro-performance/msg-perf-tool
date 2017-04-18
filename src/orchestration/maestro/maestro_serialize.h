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
#ifndef MAESTRO_SERIALIZE_H
#define MAESTRO_SERIALIZE_H

#include "maestro_note.h"
#include "msg_content_data.h"

bool maestro_serialize_note(const maestro_note_t *note, msg_content_data_t *out);

// TODO: move to a serializer module
bool maestro_note_serialize(msg_content_data_t *cont, const char *cmd);

bool maestro_note_ok_response(msg_content_data_t *cont);

// bool maestro_note_set_request(msg_content_data_t *cont, const char *opt, const char *val);
// bool maestro_note_ping_request(msg_content_data_t *cont);
// bool maestro_note_ping_response(msg_content_data_t *cont, const char *id, const char *ts);

#endif /* MAESTRO_SERIALIZER_H*/
