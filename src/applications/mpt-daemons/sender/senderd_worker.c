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
#include "senderd_worker.h"

bool started = false;
worker_options_t worker_options = {0};
static gru_list_t *children = NULL;

static void *senderd_handle_set(const maestro_note_t *request, maestro_note_t *response, 
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

		worker_options.parallel_count = (uint16_t) atoi(tmp_val);
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

		worker_options.throttle = atoi(tmp_val);
		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		
		return NULL;
	}

	logger(ERROR, "Invalid option to set: %02s", body.opt);
	maestro_note_set_cmd(response, MAESTRO_NOTE_PROTOCOL_ERROR);
	return NULL;
}


static void *senderd_handle_flush(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Flushing all buffers as requested");
	fflush(NULL);

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static void *senderd_handle_ping(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a ping request: %s", pinfo->id);

	gru_timestamp_t now = gru_time_now();

	char *safe_ts = strndup(request->payload->request.ping.ts, 
		sizeof(request->payload->request.ping.ts));

	gru_timestamp_t created = gru_time_read_str(safe_ts);
	uint64_t diff = gru_time_elapsed_milli(created, now);
	
	maestro_note_set_cmd(response, MAESTRO_NOTE_PING);
	maestro_note_ping_set_elapsed(response, diff);
	maestro_note_ping_set_id(response, pinfo->id);

	return NULL;
}

static void *senderd_handle_start(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	if (started == true || children != NULL) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
	}
	else {
		started = true;

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	}
	return NULL;
}

static void *senderd_handle_stop(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a stop request");
	started = false;

	if (children) {
		if (!abstract_worker_stop(children)) {
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			gru_list_destroy(&children);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static maestro_sheet_t *new_receiver_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);
	
	if (!ret) {	
		return NULL;
	}

	maestro_instrument_t *start_instrument = maestro_instrument_new(MAESTRO_NOTE_START, 
		senderd_handle_start, status);

	maestro_sheet_add_instrument(ret, start_instrument);

	maestro_instrument_t *stop_instrument = maestro_instrument_new(MAESTRO_NOTE_STOP, 
		senderd_handle_stop, status);

	maestro_sheet_add_instrument(ret, stop_instrument);

	maestro_instrument_t *flush_instrument = maestro_instrument_new(MAESTRO_NOTE_FLUSH, 
		senderd_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument = maestro_instrument_new(MAESTRO_NOTE_SET, 
		senderd_handle_set, status);

	maestro_sheet_add_instrument(ret, set_instrument);

	maestro_instrument_t *ping_instrument = maestro_instrument_new(MAESTRO_NOTE_PING, 
		senderd_handle_ping, status);

	maestro_sheet_add_instrument(ret, ping_instrument);


	return ret;
}

static void senderd_csv_name(const char *prefix, char *name, size_t len) 
{
	snprintf(name, len - 1, "%s-%d.csv.gz", prefix, getpid());
}

bool senderd_initialize_writer(stats_writer_t *writer, const worker_options_t *options, 
	gru_status_t *status) 
{
	csv_writer_throughput_assign(&writer->throughput);

	const options_t *prg_options = get_options_object();
	
	char tp_fname[64] = {0};
	senderd_csv_name("sender-throughput", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) prg_options->logdir;

	if (!writer->throughput.initialize(&tp_io_info, status)) {
		return false;
	}
	
	return true;
}

static bool senderd_copy_partial(worker_info_t *worker_info) {
	worker_snapshot_t snapshot = {0};
	logger_t logger = gru_logger_get();

	if (shr_buff_read(worker_info->shr, &snapshot, sizeof(worker_snapshot_t))) {
		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(INFO, "Partial summary: PID %d sent %" PRIu64 " messages in %" PRIu64
				" seconds (rate: %.2f msgs/sec)", worker_info->child,
				snapshot.count, elapsed, snapshot.throughput.rate);
	}

	return true;
}

static bool senderd_worker_execute(const vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	
	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker.options = &worker_options;

	stats_writer_t writer = {0};
	worker.writer = &writer;
	worker.name = "senderd";
	senderd_initialize_writer(worker.writer, &worker_options, &status);

	worker.can_continue = worker_check;
	
	children = abstract_worker_clone(&worker, abstract_sender_worker_start, &status);

	if (!children && !gru_status_success(&status)) {
		logger(ERROR, "Unable to initialize children: %s", status.message);

		return true;
	}
	else {
		if (!children) {
			return false;
		}
	}

	while (gru_list_count(children) > 0) {
		mpt_trace("There are still %d children running", gru_list_count(children));
		abstract_worker_watchdog(children, senderd_copy_partial); 
		
		sleep(1);
	}

	gru_list_destroy(&children);
	
	
	return true;
}


int senderd_worker_start(const options_t *options) {
	gru_status_t status = gru_status_new();
	maestro_sheet_t *sheet = new_receiver_sheet(&status);
	logger_t logger = gru_logger_get();

	if (!maestro_player_start(options, sheet, &status)) {
		logger(FATAL, "Unable to connect to maestro broker: %s\n", 
			status.message);

		return 1;
	}


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
				if (!senderd_worker_execute(&vmsl)) {
					// Child return
					break;
				}

				started = false;
			}
		}

		fflush(NULL);
	}

	return 0;
}
