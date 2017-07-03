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



void maestro_sheet_destroy(maestro_sheet_t **ptr) {
	maestro_sheet_t *sheet = *ptr;

	if (!sheet) {
		return;
	}

	gru_list_clean(sheet->instruments, maestro_instrument_destroy_wrapper);
}

void maestro_sheet_add_instrument(maestro_sheet_t *sheet,
	maestro_instrument_t *instrument) {
	gru_list_append(sheet->instruments, instrument);
}

static bool maestro_sheet_do_play(const gru_list_t *list,
	const maestro_player_info_t *pinfo,
	const maestro_note_t *request,
	maestro_note_t *response) {
	bool ret = false;
	gru_node_t *node = NULL;

	if (list == NULL) {
		return ret;
	}

	node = list->root;

	while (node) {
		maestro_instrument_t *instrument =
			gru_node_get_data_ptr(maestro_instrument_t, node);
		if (maestro_instrument_can_play(instrument, request)) {
			mpt_trace("Request and tessitura match, calling function");

			instrument->play(request, response, pinfo);

			return true;
		} else {
			node = node->next;
		}
	}

	return false;
}

void maestro_sheet_play(const maestro_sheet_t *sheet,
	const maestro_player_info_t *pinfo,
	const msg_content_data_t *req,
	msg_content_data_t *resp,
	gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Received maestro data: %s", (char *) req->data);

	maestro_note_t request = {0};
	maestro_note_t response = {0};

	maestro_note_set_type(&response, MAESTRO_TYPE_RESPONSE);


	if (!maestro_deserialize_note(req, &request, status)) {
		logger(
			ERROR, "Unable to parse request %s: %s", (char *) req->data, status->message);

		maestro_note_set_cmd(&response, MAESTRO_NOTE_PROTOCOL_ERROR);

	} else {
		if (!maestro_note_payload_prepare(&response, status)) {
			logger(WARNING, "Unable to prepare the response payload");

			maestro_note_set_cmd(&response, MAESTRO_NOTE_INTERNAL_ERROR);
		}

		if (!maestro_sheet_do_play(sheet->instruments, pinfo, &request, &response)) {
			maestro_note_set_cmd(&response, MAESTRO_NOTE_INTERNAL_ERROR);
		}
	}

	maestro_serialize_note(&response, resp);
	maestro_note_payload_cleanup(&response);
	maestro_note_payload_cleanup(&request);
}