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
#include <ctype.h>
#include <stdint.h>

#include <common/gru_status.h>

#include "msg_content_data.h"

#define MAESTRO_NOTE_TYPE_LENGTH 1
#define MAESTRO_NOTE_CMD_LENGTH 2
#define MAESTRO_NOTE_PAYLOAD_MAX_LENGTH 216

#define MAESTRO_HEADER_SIZE (MAESTRO_NOTE_TYPE_LENGTH + MAESTRO_NOTE_CMD_LENGTH)
#define MAESTRO_NOTE_SIZE (MAESTRO_HEADER_SIZE + MAESTRO_NOTE_PAYLOAD_MAX_LENGTH)

#define MAESTRO_TYPE_REQUEST '0'
#define MAESTRO_TYPE_RESPONSE '1'

/** Start execution */
#define MAESTRO_NOTE_START "01"
/** Stop execution **/
#define MAESTRO_NOTE_STOP "02"
/** Flush all buffers */
#define MAESTRO_NOTE_FLUSH "03"

/** Set options */
#define MAESTRO_NOTE_SET "04"

/** Lengh of the opt field in a set note */
#define MAESTRO_NOTE_OPT_LEN 2

/** Lengh of the value field in a set note */
#define MAESTRO_NOTE_OPT_VALUE_LEN (MAESTRO_NOTE_PAYLOAD_MAX_LENGTH - MAESTRO_NOTE_OPT_LEN)

#define MAESTRO_NOTE_PING "10"

/** Response OK */
#define MAESTRO_NOTE_OK "E0"

/** Protocol error response */
#define MAESTRO_NOTE_PROTOCOL_ERROR "F0"

/** Internal server error */
#define MAESTRO_NOTE_INTERNAL_ERROR "F1"

/** Set broker address */
#define MAESTRO_NOTE_OPT_SET_BROKER "00"

/** Set duration type (count or duration). Values are defined as parameters to the message */
#define MAESTRO_NOTE_OPT_SET_DURATION_TYPE "01"

/** Set the log level */
#define MAESTRO_NOTE_OPT_SET_LOG_LEVEL "02"

/** Set the parallel count */
#define MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT "03"

/** Set a fixed message size */
#define MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE "04"

/** Set throtle */
#define MAESTRO_NOTE_OPT_SET_THROTTLE "05"

typedef struct maestro_payload_ping_request_t_ {
	char ts[18];
} maestro_payload_ping_request_t;

typedef struct maestro_payload_ping_reply_t_ {
	char id[36];
	char elapsed[18];
} maestro_payload_ping_reply_t;


typedef struct maestro_payload_set_t_ {
	char opt[MAESTRO_NOTE_OPT_LEN];
	char value[MAESTRO_NOTE_OPT_VALUE_LEN];
} maestro_payload_set_t;

typedef union maestro_payload_t_ {
	union {
		maestro_payload_ping_reply_t ping;
	} response;
	union {
		maestro_payload_set_t set;
		maestro_payload_ping_request_t ping;
	} request;
} maestro_payload_t;

typedef struct maestro_note_t_ {
	char type;
	char command[2];
	maestro_payload_t *payload;
} maestro_note_t;


/** 
 * Prepare a note for receiving a payload (ie.: allocate memory for the payload)
 * @return true if successfull of false otherwise
 */
bool maestro_note_payload_prepare(maestro_note_t *note, gru_status_t *status);

/**
 * Frees memory used by a payload
 */
void maestro_note_payload_cleanup(maestro_note_t *note);

/**
 * Parse a note
 * @param data data to parse 
 * @param size data size 
 * @param note output note 
 * @param status status structure in case of error 
 * @return true if successfully parsed or false otherwise
 */
bool maestro_note_parse(const void *data, size_t size, maestro_note_t *note, 
	gru_status_t *status);
bool maestro_note_equals(const maestro_note_t *note, const char *cmd);

void maestro_note_ping_set_id(maestro_note_t *note, const char *id);
void maestro_note_ping_set_ts(maestro_note_t *note, const char *ts);
void maestro_note_ping_set_elapsed(maestro_note_t *note, uint64_t ts);

void maestro_note_set_type(maestro_note_t *note, const char type);
void maestro_note_set_cmd(maestro_note_t *note, const char *cmd);

void maestro_note_set_opt(maestro_note_t *note, const char *opt, const char *value);





#endif /* MAESTRO_NOTE_H */
