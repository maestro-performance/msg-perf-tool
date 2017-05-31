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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <common/gru_status.h>

#include "msg_content_data.h"

#define MAESTRO_NOTE_TYPE_LENGTH 1
#define MAESTRO_NOTE_CMD_LENGTH 2
#define MAESTRO_NOTE_PAYLOAD_MAX_LENGTH 253

#define MAESTRO_HEADER_SIZE (MAESTRO_NOTE_TYPE_LENGTH + MAESTRO_NOTE_CMD_LENGTH)
#define MAESTRO_NOTE_SIZE (MAESTRO_HEADER_SIZE + MAESTRO_NOTE_PAYLOAD_MAX_LENGTH)

#define MAESTRO_TYPE_REQUEST '0'
#define MAESTRO_TYPE_RESPONSE '1'

typedef enum maestro_command_t_ {
	/** Receiver execution **/
	MAESTRO_NOTE_START_RECEIVER,
	MAESTRO_NOTE_STOP_RECEIVER,
	/** Sender execution */
	MAESTRO_NOTE_START_SENDER,
	MAESTRO_NOTE_STOP_SENDER,
	/** Inspector execution **/
	MAESTRO_NOTE_START_INSPECTOR,
	MAESTRO_NOTE_STOP_INSPECTOR,
	MAESTRO_NOTE_FLUSH,
	MAESTRO_NOTE_SET,
	MAESTRO_NOTE_STATS,
	MAESTRO_NOTE_HALT,
	MAESTRO_NOTE_PING,
	MAESTRO_NOTE_OK,
	MAESTRO_NOTE_PROTOCOL_ERROR,
	MAESTRO_NOTE_INTERNAL_ERROR
} maestro_command_t;


/** Lengh of the opt field in a set note */
#define MAESTRO_NOTE_OPT_LEN 2

#define MAESTRO_NOTE_OPT_VALUE_LEN                                                       \
       (MAESTRO_NOTE_PAYLOAD_MAX_LENGTH - MAESTRO_NOTE_OPT_LEN)



typedef enum set_opts_t_ {
	/** Broker address */
	MAESTRO_NOTE_OPT_SET_BROKER = 0,
	/** Duration type (count or duration).
	 * Values are defined as parameters to the message
 	*/
	MAESTRO_NOTE_OPT_SET_DURATION_TYPE,
	/** Set the log level */
	MAESTRO_NOTE_OPT_SET_LOG_LEVEL,
	/** Set the parallel count */
	MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT,
	/** Set message size */
	MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE,
	/** Set throtle */
	MAESTRO_NOTE_OPT_SET_THROTTLE
} set_opts_t;


#define MAESTRO_CLIENT_ID_SIZE 36

#define MAESTRO_CLIENT_NAME_SIZE 72

typedef struct maestro_payload_ping_request_t_ {
	char *ts;
} maestro_payload_ping_request_t;

typedef struct maestro_payload_ping_reply_t_ {
	char *id;
	char *name;
  	uint64_t elapsed;
} maestro_payload_ping_reply_t;

typedef struct maestro_payload_stats_perf_t_ {
	char timestamp[GRU_TS_STRING_MAX];
	char count[24];
	char rate[10];
	char latency[16];
} maestro_payload_stats_perf_t;

typedef struct maestro_payload_stats_reply_t_ {
	char id[MAESTRO_CLIENT_ID_SIZE];
	char name[MAESTRO_CLIENT_NAME_SIZE];
	char child_count[5];
	char role[10]; // sender / receiver / jmonitor / nmonitor /
	char roleinfo[8]; // ie: node number on the cluster, etc
	char stat_type; // perf (lat/tp), other
	union {
		maestro_payload_stats_perf_t perf;
	} stats;
} maestro_payload_stats_reply_t;

typedef struct maestro_payload_set_t_ {
	int64_t opt;
	char *value;
} maestro_payload_set_t;

typedef union maestro_payload_t_ {
	union {
		maestro_payload_ping_reply_t ping;
		maestro_payload_stats_reply_t stats;
	} response;
	union {
		maestro_payload_set_t set;
		maestro_payload_ping_request_t ping;
	} request;
} maestro_payload_t;

typedef struct maestro_note_t_ {
	char type;
	int64_t command;
	maestro_payload_t *payload;
} maestro_note_t;

#define maestro_set_payload_txt_field(field, source)                                     \
	snprintf(field, (int) sizeof(field), "%-*s", ((int) sizeof(field) - 1), source)

#define maestro_set_payload_uint32_field(field, value)                                   \
	sprintf(field, "%-*" PRIu32 "", (int) sizeof(field), value)

#define maestro_set_payload_uint64_field(field, value)                                   \
	sprintf(field, "%-*" PRIu64 "", (int) sizeof(field), value)

#define maestro_set_payload_double_field(field, value)                                   \
	sprintf(field, "%-*.2f", (int) sizeof(field), value)

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
 * Sets the client ID in the ping response
 */
void maestro_note_ping_set_id(maestro_note_t *note, const char *id);

/**
 * Sets the client name in the ping response
 */
void maestro_note_ping_set_name(maestro_note_t *note, const char *name);

/**
 * Sets the timestamp in the ping request
 */
void maestro_note_ping_set_ts(maestro_note_t *note, const char *ts);

/**
 * Sets the elapsed time in the ping response
 */
void maestro_note_ping_set_elapsed(maestro_note_t *note, uint64_t ts);

void maestro_note_set_type(maestro_note_t *note, const char type);
void maestro_note_set_cmd(maestro_note_t *note, maestro_command_t cmd);

void maestro_note_set_opt(maestro_note_t *note, int64_t opt, const char *value);

/**
 * Sets the ID in the stats response
 */
void maestro_note_stats_set_id(maestro_note_t *note, const char *id);

/**
 * Sets the name in the stats response
 */
void maestro_note_stats_set_name(maestro_note_t *note, const char *name);

/**
 * Sets the child count in the stats response
 */
void maestro_note_stats_set_child_count(maestro_note_t *note, uint32_t count);

/**
 * Sets the role in the stats response
 */
void maestro_note_stats_set_role(maestro_note_t *note, const char *role);

/**
 * Sets the role info in the stats response
 */
void maestro_note_stats_set_roleinfo(maestro_note_t *note, const char *roleinfo);

/**
 * Sets the stat type in the stats response
 */
void maestro_note_stats_set_stat_type(maestro_note_t *note, const char stat_type);

/**
 * Sets the timestamp for the performance statistics
 */
void maestro_note_stats_set_perf_ts(maestro_note_t *note, const char *ts);

/**
 * Sets the message count for the performance statistics
 */
void maestro_note_stats_set_perf_count(maestro_note_t *note, uint64_t count);

/**
 * Sets the average throughput rate for the performance statistics
 */
void maestro_note_stats_set_perf_rate(maestro_note_t *note, double rate);

/**
 * Sets the average latency for the performance statistics
 */
void maestro_note_stats_set_perf_latency(maestro_note_t *note, double latency);

#endif /* MAESTRO_NOTE_H */
