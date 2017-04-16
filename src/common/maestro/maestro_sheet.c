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
#include "maestro_sheet.h"

maestro_sheet_t *maestro_sheet_new(const char *location, gru_status_t *status) {
	maestro_sheet_t *ret = gru_alloc(sizeof(maestro_sheet_t), status);
	if (!ret) {
		return NULL;
	}

	ret->instruments = gru_list_new(status); 
	if (!ret->instruments) {
		gru_dealloc((void **) &ret);

		return NULL;
	}


	ret->location = strdup(location);
	if (!ret->location) {
		gru_list_destroy(&ret->instruments);
		gru_dealloc((void **) &ret);

		return NULL;
	}

	return ret;
}

void maestro_sheet_add_instrument(maestro_sheet_t *sheet, 
	maestro_instrument_t *instrument)
{
	gru_list_append(sheet->instruments, instrument);

}

typedef struct maestro_exchange_t_ {
	maestro_note_t request;
	maestro_note_t response;
} maestro_exchange_t;

static void maestro_sheet_do_play(const void *nodedata, void *payload) {
	maestro_exchange_t *exchange = (maestro_exchange_t *) payload;
	maestro_instrument_t *instrument = (maestro_instrument_t *) nodedata;

	if (strncmp(instrument->tessitura.command, exchange->request.command, MAESTRO_NOTE_CMD_LENGTH) == 0) {
		mpt_trace("Request and tessitura match, calling function");

		instrument->play(&exchange->request, &exchange->response);
	}
	else {
		mpt_trace("Request %03s is unkown, therefore ignoring (current = %s)", 
			exchange->request.command, instrument->tessitura.command);
	}

}

void maestro_sheet_play(const maestro_sheet_t *sheet, const msg_content_data_t *req, 
	msg_content_data_t *resp, gru_status_t *status)
{
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Received maestro data: %s", (char *) req->data);

	maestro_exchange_t exchange = {0};

	if (!maestro_note_parse(req->data, req->size, &exchange.request, status)) {
		logger(ERROR, "Unable to parse request %s: %s", (char *) req->data, 
			status->message);

		maestro_note_protocol_error_response(resp);
		
		return;
	}
		
	gru_list_for_each(sheet->instruments, maestro_sheet_do_play, &exchange);
	maestro_note_ok_response(resp);
}