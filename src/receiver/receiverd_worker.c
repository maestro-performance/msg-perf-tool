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
#include "receiverd_worker.h"

bool can_start = false;

static void *receiverd_handle_set(maestro_note_t *request, maestro_note_t *response) {
	logger_t logger = gru_logger_get();

	maestro_note_body_set_t *body = request->payload;

	logger(INFO, "Setting option: %s to %s", body->opt, body->value);
	fflush(NULL);
}


static void *receiverd_handle_flush(maestro_note_t *request, maestro_note_t *response) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Flushing all buffers as requested");
	fflush(NULL);
}

static void *receiverd_handle_ping(maestro_note_t *request, maestro_note_t *response) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a ping request");
	
	
	return NULL;
}

static void *receiverd_handle_start(maestro_note_t *request, maestro_note_t *response) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	can_start = true;
	return NULL;
}

static maestro_sheet_t *new_receiver_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);
	
	if (!ret) {	
		return NULL;
	}

	maestro_instrument_t *start_instrument = maestro_instrument_new(MAESTRO_NOTE_START, 
		receiverd_handle_start, status);

	maestro_sheet_add_instrument(ret, start_instrument);

	maestro_instrument_t *flush_instrument = maestro_instrument_new(MAESTRO_NOTE_FLUSH, 
		receiverd_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument = maestro_instrument_new(MAESTRO_NOTE_SET, 
		receiverd_handle_set, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	return ret;
}

int receiverd_worker_start(const options_t *options) {
	gru_status_t status = gru_status_new();
	maestro_sheet_t *sheet = new_receiver_sheet(&status);

	if (!maestro_player_start(options, sheet, &status)) {
		fprintf(stderr, "Unable to connect to maestro broker: %s\n", 
			status.message);

		return;
	}

	while (true) {
		sleep(1);
	}
}