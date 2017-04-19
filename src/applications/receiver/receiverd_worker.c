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

bool started = false;
worker_options_t worker_options = {0};

static void *receiverd_handle_set(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	

	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	maestro_payload_set_t body = request->payload->request.set;

	logger(INFO, "Setting option: %.%s to %.*s", (int) sizeof(body.opt), body.opt, 
		(int) sizeof(body.value), body.value);

	char tmp_opt[MAESTRO_NOTE_OPT_LEN + 1] = {0};
	char tmp_val[MAESTRO_NOTE_OPT_VALUE_LEN + 1] = {0};

	strncpy(tmp_opt, body.opt, sizeof(body.opt));
	strncpy(tmp_val, body.value, sizeof(body.value));

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_BROKER, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting broker to: %s", tmp_val);

		worker_options.uri = gru_uri_parse(tmp_val, &status);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_DURATION_TYPE, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting duration option");

		worker_options.duration_type = MESSAGE_COUNT;
		worker_options.duration.count = atol(tmp_val);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_LOG_LEVEL, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting log-level option");

		worker_options.log_level = gru_logger_get_level(tmp_val);
		gru_logger_set_mininum(worker_options.log_level);

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting parallel count option");

		worker_options.parallel_count = atoi(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting message size option");

		worker_options.message_size = atol(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		return NULL;
	}

	if (strncmp(body.opt, MAESTRO_NOTE_OPT_SET_THROTTLE, MAESTRO_NOTE_OPT_LEN) == 0) {
		logger(INFO, "Setting throttle option");

		worker_options.throttle = atol(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		
		return NULL;
	}

	logger(ERROR, "Invalid option to set: %02s", body.opt);
	maestro_note_set_cmd(response, MAESTRO_NOTE_PROTOCOL_ERROR);
	return NULL;
}


static void *receiverd_handle_flush(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Flushing all buffers as requested");
	fflush(NULL);

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static void *receiverd_handle_ping(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a ping request: %s", pinfo->id);

	gru_timestamp_t now = gru_time_now();

	char *safe_ts = strndup(request->payload->request.ping.ts, 
		sizeof(request->payload->request.ping.ts));

	gru_timestamp_t created = gru_time_read_str(safe_ts);
	uint64_t diff = statistics_diff(created, now);
	
	maestro_note_set_cmd(response, MAESTRO_NOTE_PING);
	maestro_note_ping_set_elapsed(response, diff);
	maestro_note_ping_set_id(response, pinfo->id);

	return NULL;
}

static void *receiverd_handle_start(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	started = true;

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static void *receiverd_handle_stop(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a stop request");
	started = false;

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
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

	maestro_instrument_t *stop_instrument = maestro_instrument_new(MAESTRO_NOTE_STOP, 
		receiverd_handle_stop, status);

	maestro_sheet_add_instrument(ret, stop_instrument);

	maestro_instrument_t *flush_instrument = maestro_instrument_new(MAESTRO_NOTE_FLUSH, 
		receiverd_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument = maestro_instrument_new(MAESTRO_NOTE_SET, 
		receiverd_handle_set, status);

	maestro_sheet_add_instrument(ret, set_instrument);

	maestro_instrument_t *ping_instrument = maestro_instrument_new(MAESTRO_NOTE_PING, 
		receiverd_handle_ping, status);

	maestro_sheet_add_instrument(ret, ping_instrument);


	return ret;
}

void receiverd_run_test(const vmsl_t *vmsl, const worker_options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	const uint32_t tp_interval = 10;
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0}; 

	stat_io_t *stat_io = statistics_init(RECEIVER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);
		gru_status_reset(&status);

		return;
	}

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_RECEIVER, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_DEFAULT,
	};

	msg_conn_info_gen_id(&opt.conn_info);
	opt.uri = options->uri;

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, opt, NULL, &status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	vmsl_stat_t ret = vmsl->subscribe(msg_ctxt, NULL, &status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	msg_content_data_init(&content_storage, options->message_size, &status);
	if (!gru_status_success(&status)) {
		goto err_exit;
	}

	gru_timestamp_t last;
	gru_timestamp_t start = gru_time_now();
	time_t last_calc = 0;

	statistics_latency_header(stat_io);
	statistics_throughput_header(stat_io);

	install_interrupt_handler();

	while (started) {
		vmsl_stat_t rstat = vmsl->receive(msg_ctxt, &content_storage, &status);
		if (unlikely(vmsl_stat_error(rstat))) {
			logger(ERROR, "Error receiving data: %s\n", status.message);

			gru_status_reset(&status);
			break;
		}

		last = gru_time_now();

		if (last_calc <= (last.tv_sec - tp_interval)) {
			uint64_t processed_count = content_storage.count - last_count;

			statistics_throughput_partial(stat_io, last, tp_interval, processed_count);
			
			last_count = content_storage.count;
			last_calc = last.tv_sec;
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	statistics_destroy(&stat_io);

	uint64_t elapsed = statistics_diff(start, last);
	double rate = ((double) content_storage.count / (double) elapsed) * 1000;

	uint64_t total_received = content_storage.count;

	logger(STAT,
		"summary;received;%" PRIu64 ";elapsed;%" PRIu64 ";rate;%.2f",
		total_received,
		elapsed,
		rate);

	logger(INFO,
		"Summary: received %" PRIu64 " messages in %" PRIu64
		" milliseconds (rate: %.2f msgs/sec)",
		total_received,
		elapsed,
		rate);
	logger(INFO, "Errors: received %" PRIu64, content_storage.errors);

	msg_content_data_release(&content_storage);
	return;

err_exit:
	fprintf(stderr, "%s", status.message);
	statistics_destroy(&stat_io);
	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		vmsl->destroy(msg_ctxt, &status);
	}

	gru_status_reset(&status);
	return;
}

int receiverd_worker_start(const options_t *options) {
	gru_status_t status = gru_status_new();
	maestro_sheet_t *sheet = new_receiver_sheet(&status);

	if (!maestro_player_start(options, sheet, &status)) {
		fprintf(stderr, "Unable to connect to maestro broker: %s\n", 
			status.message);

		return 1;
	}

	logger_t logger = gru_logger_get();
	while (true) {
		sleep(1);

		if (started) {
			vmsl_t vmsl = vmsl_init();

			if (!vmsl_assign_by_url(&worker_options.uri, &vmsl)) {
				char *uri = gru_uri_simple_format(&worker_options.uri, &status); 

				if (!uri) {
					logger(ERROR, "Unable to assign a VMSL: %s", status.message);
				}
				else { 
					logger(ERROR, "Unable to assign a VMSL for the URI: %s", uri);
					gru_dealloc_string(&uri);
				}
			}
			else {
				receiverd_run_test(&vmsl, &worker_options);
			}
		}

		fflush(NULL);
	}

	return 0;
}