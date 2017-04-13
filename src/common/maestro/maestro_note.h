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
#ifndef MAESTRO_NOTE_H
#define MAESTRO_NOTE_H

#include <stdlib.h>
#include <stdbool.h>

#include <common/gru_status.h>

#include "msg_content_data.h"

#define MAESTRO_NOTE_TYPE_LENGTH 1
#define MAESTRO_NOTE_CMD_LENGTH 2
#define MAESTRO_NOTE_PAYLOAD_MAX_LENGTH 252

#define MAESTRO_HEADER_SIZE (MAESTRO_NOTE_TYPE_LENGTH + MAESTRO_NOTE_CMD_LENGTH)
#define MAESTRO_NOTE_SIZE (MAESTRO_NOTE_TYPE_LENGTH + MAESTRO_NOTE_CMD_LENGTH + MAESTRO_NOTE_PAYLOAD_MAX_LENGTH)

#define MAESTRO_TYPE_REQUEST "0"
#define MAESTRO_TYPE_RESPONSE "1"

#define maestro_request(maestro_note__) (MAESTRO_TYPE_REQUEST maestro_note__)
#define maestro_response(maestro_note__) (MAESTRO_TYPE_RESPONSE maestro_note__)



/** Start execution */
#define MAESTRO_NOTE_START "01"
/** Stop execution **/
#define MAESTRO_NOTE_STOP "02"
/** Flush all buffers */
#define MAESTRO_NOTE_FLUSH "03" 

#define MAESTRO_NOTE_PING "10"

#define MAESTRO_NOTE_OK "E0"

#define MAESTRO_NOTE_PROTOCOL_ERROR "F0"


typedef struct maestro_note_t_ {
	char type;
	char command[3]; 
	void *payload;
} maestro_note_t;

bool maestro_note_parse(const void *data, size_t size, maestro_note_t *note, 
	gru_status_t *status);
bool maestro_note_serialize(msg_content_data_t *cont, const char *cmd);




#endif /* MAESTRO_NOTE_H */
