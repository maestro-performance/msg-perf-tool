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
#include "brokerd_worker.h"

bool started = false;
bool halt = false;

worker_options_t worker_options = {0};

static void *brokerd_handle_set(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo)
{
	return commond_handle_set(request, response, &worker_options);
}

static void *brokerd_handle_flush(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	commond_handle_flush(request, response, pinfo);
	if (started) {
		gru_status_t status = gru_status_new();
		if (!bmic_writer_flush(&status)) {
			logger(ERROR, "Unable to flush bmic data to disk");
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}


static void *brokerd_handle_start(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	if (started == true) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
	} else {
		started = true;

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static void *brokerd_handle_stop(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a stop request");
	started = false;

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static void *brokerd_handle_halt(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a halt request");
	brokerd_handle_stop(request, response, pinfo);
	halt = true;
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *brokerd_handle_stats(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	return NULL;
}

static maestro_sheet_t *new_receiver_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);

	if (!ret) {
		return NULL;
	}

	maestro_instrument_t *start_instrument =
		maestro_instrument_new(MAESTRO_NOTE_START, brokerd_handle_start, status);

	maestro_sheet_add_instrument(ret, start_instrument);

	maestro_instrument_t *stop_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STOP, brokerd_handle_stop, status);

	maestro_sheet_add_instrument(ret, stop_instrument);

	maestro_instrument_t *flush_instrument =
		maestro_instrument_new(MAESTRO_NOTE_FLUSH, brokerd_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument =
		maestro_instrument_new(MAESTRO_NOTE_SET, brokerd_handle_set, status);

	maestro_sheet_add_instrument(ret, set_instrument);

	maestro_instrument_t *ping_instrument =
		maestro_instrument_new(MAESTRO_NOTE_PING, commond_handle_ping, status);

	maestro_sheet_add_instrument(ret, ping_instrument);

	maestro_instrument_t *stats_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STATS, brokerd_handle_stats, status);

	maestro_sheet_add_instrument(ret, stats_instrument);

	maestro_instrument_t *halt_instrument =
		maestro_instrument_new(MAESTRO_NOTE_HALT, brokerd_handle_halt, status);

	maestro_sheet_add_instrument(ret, halt_instrument);

	return ret;
}

static bool brokerd_abort_check(bmic_queue_stat_t *qstats) {
	if (worker_options.duration_type == MESSAGE_COUNT) {
		if ((qstats->msg_ack_count + qstats->msg_exp_count) >=
			worker_options.duration.count) {
			return true;
		}
	} else {
		gru_timestamp_t now = gru_time_now();

		if (now.tv_sec >= worker_options.duration.time.end.tv_sec) {
			return true;
		}
	}

	return false;
}

static bool brokerd_gen_unique_name(char *filename, size_t size, gru_status_t *status) {
	gru_timestamp_t now = gru_time_now();
	char *curr_time_str = gru_time_write_format(&now, "%Y%m%d%H%M%S", status);
	if (!curr_time_str) {
		return false;
	}

	snprintf(
		filename, size, "broker-jvm-inspector-%d-%s.csv.gz", getpid(), curr_time_str);
	gru_dealloc_string(&curr_time_str);

	return true;
}

static bool brokerd_collect(gru_status_t *status) {
	logger_t logger = gru_logger_get();
	bmic_context_t ctxt = {0};

	char filename[64] = {0};
	if (!brokerd_gen_unique_name(filename, sizeof(filename), status)) {
		return false;
	}

	const options_t *options = get_options_object();
	if (!bmic_writer_initialize(options->logdir, filename, status)) {
		return false;
	}

	logger(INFO, "Initializing BMIC context");
	if (!mpt_init_bmic_ctxt(worker_options.uri, &ctxt, status)) {
		return false;
	}

	logger(INFO, "Purging the queue and reseting the counters");
	if (!mpt_purge_queue(&ctxt, &worker_options.uri.path[1], status)) {
		return false;
	}

	bmic_api_interface_t *api = ctxt.api;
	bmic_java_info_t jinfo = api->java.java_info(ctxt.handle, status);

	while (started) {
		if (!bmic_writer_current_time(status)) {
			logger(ERROR, "Unable to write current time: %s", status->message);

			break;
		}

		bmic_java_os_info_t osinfo = api->java.os_info(ctxt.handle, status);
		bmic_writer_osinfo(&osinfo);

		mpt_java_mem_t java_mem = {0};
		mpt_get_mem_info(&ctxt, jinfo.memory_model, &java_mem, status);
		if (gru_status_error(status)) {
			logger(ERROR, "%s", status->message);
			break;
		}

		bmic_writer_java_mem(&java_mem, jinfo.memory_model);

		bmic_queue_stat_t qstats = {0};
		mpt_get_queue_stats(&ctxt, &worker_options.uri.path[1], &qstats, status);

		if (gru_status_error(status)) {
			logger(ERROR, "%s", status->message);
			break;
		}
		bmic_writer_queue_stat(&qstats);

		if (brokerd_abort_check(&qstats)) {
			break;
		}

		bmic_writer_flush(status);
		sleep(10);
	}

	if (!gru_status_success(status)) {
		bmic_writer_finalize(status);
		bmic_context_cleanup(&ctxt);
		logger(INFO, "Broker inspector completed the inspection with errors");

		return false;
	}


	bmic_writer_finalize(status);
	bmic_context_cleanup(&ctxt);
	logger(INFO, "Broker inspector completed the inspection");

	return true;
}

int brokerd_worker_start(const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	maestro_sheet_t *sheet = new_receiver_sheet(&status);

	if (!maestro_player_start(options, sheet, &status)) {
		logger(FATAL, "Unable to connect to maestro broker: %s\n", status.message);

		return 1;
	}

	while (!halt) {
		sleep(1);

		if (started) {
			if (!brokerd_collect(&status)) {
				logger(ERROR, "Unable to collect broker data: %s", status.message);
			}

			started = false;
		}

		fflush(NULL);
	}

	return 0;
}