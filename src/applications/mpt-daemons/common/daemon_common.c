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

	logger(INFO,
		"Setting option: %.%s to %.*s",
		(int) sizeof(body.opt),
		body.opt,
		(int) sizeof(body.value),
		body.value);

	char tmp_opt[MAESTRO_NOTE_OPT_LEN + 1] = {0};
	char tmp_val[MAESTRO_NOTE_OPT_VALUE_LEN + 1] = {0};

	strncpy(tmp_opt, body.opt, sizeof(body.opt));
	strncpy(tmp_val, body.value, sizeof(body.value));

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_BROKER, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting broker to: %s", tmp_val);

		worker_options->uri = gru_uri_parse(tmp_val, &status);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_DURATION_TYPE, MAESTRO_NOTE_OPT_LEN) ==
		0) {
		logger(INFO, "Setting duration option");

		gru_duration_t duration = gru_duration_new();
		if (!gru_duration_parse(&duration, tmp_val)) {
			worker_options->duration_type = MESSAGE_COUNT;
			worker_options->duration.count = atol(tmp_val);
		} else {
			worker_options->duration_type = TEST_TIME;
			worker_options->duration.time = duration;
		}

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_LOG_LEVEL, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting log-level option");

		worker_options->log_level = gru_logger_get_level(tmp_val);
		gru_logger_set_mininum(worker_options->log_level);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT, MAESTRO_NOTE_OPT_LEN) ==
		0) {
		logger(INFO, "Setting parallel count option");

		worker_options->parallel_count = (uint16_t) atoi(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting message size option");

		worker_options->message_size = atol(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_THROTTLE, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting throttle option");

		worker_options->throttle = atoi(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	logger(ERROR, "Invalid option to set: %02s", body.opt);
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

	logger(INFO, "Ping request: %s", pinfo->id);

	gru_timestamp_t now = gru_time_now();

	char *safe_ts = strndup(
		request->payload->request.ping.ts, sizeof(request->payload->request.ping.ts));

	gru_timestamp_t created = gru_time_read_str(safe_ts);
	uint64_t diff = gru_time_elapsed_milli(created, now);

	maestro_note_set_cmd(response, MAESTRO_NOTE_PING);
	maestro_note_ping_set_elapsed(response, diff);
	maestro_note_ping_set_id(response, pinfo->id);

	return NULL;
}