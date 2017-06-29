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
 #include "daemon_common.h"

void *commond_handle_set(const maestro_note_t *request, maestro_note_t *response,
	worker_options_t *worker_options)
{
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	maestro_payload_set_t body = request->payload->request.set;

	logger(DEBUG, "Handling a set option request");


	if (body.opt == MAESTRO_NOTE_OPT_SET_BROKER) {
		worker_options->uri = gru_uri_parse(body.value, &status);
		if (gru_status_error(&status)) {
			logger(ERROR, "Unable to set broker setting: %s", status.message);
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_DURATION_TYPE) {
		logger(INFO, "Setting duration option");

		gru_duration_t duration = gru_duration_new();
		if (!gru_duration_parse(&duration, body.value)) {
			worker_options->duration_type = MESSAGE_COUNT;
			worker_options->duration.count = atol(body.value);
		} else {
			worker_options->duration_type = TEST_TIME;
			worker_options->duration.time = duration;
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_LOG_LEVEL) {
		logger(INFO, "Setting log-level option");

		worker_options->log_level = gru_logger_get_level(body.value);
		gru_logger_set_mininum(worker_options->log_level);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT) {
		logger(INFO, "Setting parallel count option");

		worker_options->parallel_count = (uint16_t) atoi(body.value);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE) {
		logger(INFO, "Setting message size option");

		if (body.value[0] == '~') {
			worker_options->message_size = atoi(body.value + 1);
			worker_options->variable_size = true;
		}
		else {
			worker_options->message_size = atoi(body.value);
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_THROTTLE) {
		logger(INFO, "Setting throttle option");

		worker_options->throttle = atoi(body.value);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	logger(ERROR, "Invalid option to set: %d", body.opt);
	maestro_note_set_cmd(response, MAESTRO_NOTE_PROTOCOL_ERROR);
	return NULL;
}

void *commond_handle_flush(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Flushing all buffers as requested");
	fflush(NULL);

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

void *commond_handle_ping(const maestro_note_t *request, maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Ping request: %s / %s", pinfo->id, pinfo->name);

	gru_timestamp_t now = gru_time_now();

	gru_timestamp_t created = {0};
	created.tv_sec = request->payload->request.ping.sec;
	created.tv_usec = request->payload->request.ping.usec;

	uint64_t diff = gru_time_elapsed_milli(created, now);

	maestro_note_set_cmd(response, MAESTRO_NOTE_PING);
	maestro_note_ping_set_id(response, pinfo->id);
	maestro_note_ping_set_name(response, pinfo->name);
	maestro_note_ping_set_elapsed(response, diff);

	return NULL;
}