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
#include <worker_options.h>
#include "daemon_common.h"

void *commond_handle_set(const maestro_note_t *request, maestro_note_t *response,
	worker_options_t *worker_options)
{
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	maestro_payload_set_t body = request->payload->request.set;

	if (body.opt == MAESTRO_NOTE_OPT_SET_BROKER) {
		logger(INFO, "Set broker URL to %s", body.value);

		worker_options->uri = gru_uri_parse(body.value, &status);
		if (gru_status_error(&status)) {
			logger(ERROR, "Unable to set broker setting: %s", status.message);
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_DURATION_TYPE) {
		gru_duration_t duration = gru_duration_new();
		if (!gru_duration_parse(&duration, body.value)) {
			worker_options->duration_type = MESSAGE_COUNT;
			worker_options->duration.count = atol(body.value);

			logger(INFO, "Set duration count to %"PRIu64"",
				   worker_options->duration.count);
		} else {
			worker_options->duration_type = TEST_TIME;
			worker_options->duration.time = duration;

			logger(INFO, "Set duration time to %s", body.value);
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_LOG_LEVEL) {
		worker_options->log_level = gru_logger_get_level(body.value);
		gru_logger_set_mininum(worker_options->log_level);
		logger(INFO, "Set log-level to %s", body.value);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT) {
		worker_options->parallel_count = (uint16_t) atoi(body.value);

		logger(INFO, "Set parallel count to %"PRIu16"", worker_options->parallel_count);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE) {
		if (body.value[0] == '~') {
			worker_options->message_size = atoi(body.value + 1);
			worker_options->variable_size = true;

			logger(INFO, "Set variable message size to %s", body.value);
		}
		else {
			worker_options->message_size = atoi(body.value);

			logger(INFO, "Set fixed message size to %"PRId64"",
				   worker_options->message_size);
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_THROTTLE) {
		worker_options->throttle = atoi(body.value);

		logger(INFO, "Set throttle to %"PRIu32"", worker_options->throttle);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_SET_RATE) {
		worker_options->rate = atoi(body.value);

		logger(INFO, "Set rate to %"PRIu32"", worker_options->rate);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (body.opt == MAESTRO_NOTE_OPT_FCL) {
		worker_options->condition.latency = atoi(body.value);

		logger(INFO, "Set fcl (fail-condition-latency) to %"PRId64"",
			   worker_options->condition.latency);
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

	logger(DEBUG, "Creation seconds.nano: %"PRIu64".%"PRIu64"",
		   created.tv_sec, created.tv_usec);

	uint64_t diff = gru_time_elapsed_milli(created, now);

	maestro_note_set_cmd(response, MAESTRO_NOTE_PING);
	maestro_note_ping_set_elapsed(response, diff);

	return NULL;
}