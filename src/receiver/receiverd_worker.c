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

static void *receiverd_handle_ping(maestro_note_t *note, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a ping request");
	
	
	return NULL;
}

static void *receiverd_handle_start(maestro_note_t *note, gru_status_t *status) {
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

	maestro_instrument_t *instrument = maestro_instrument_new(MAESTRO_NOTE_START, 
		receiver_handle_start, status);

	maestro_sheet_add_instrument(ret, instrument);

	return ret;
}

int receiverd_start(const options_t *options) {
	maestro_sheet_t *sheet = new_receiver_sheet(&status);

	if (!maestro_player_start(options, sheet, &status)) {
		fprintf(stderr, "Unable to connect to maestro broker: %s\n", 
			status.message);

		return;
	}

	while 

}