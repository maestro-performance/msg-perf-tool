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
#include <maestro/maestro_note.h>
#include "receiverd_worker.h"

bool started = false;
bool halt = false;

worker_options_t worker_options = {0};
static char *locations[] = MAESTRO_RECEIVER_DAEMON_SHEETS;

static void *receiverd_handle_set(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo)
{
	return commond_handle_set(request, response, &worker_options);
}


static void *receiverd_handle_start(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a start request");
	if (started == true || worker_list_is_running()) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
	} else {
		started = true;

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	}

	return NULL;
}

static void *receiverd_handle_stop(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a stop request");
	started = false;

	if (worker_list_is_running()) {
		if (!worker_manager_stop()) {
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *receiverd_handle_test_failed(const maestro_note_t *request,
								   maestro_note_t *response,
								   const maestro_player_info_t *pinfo) {
	started = false;

	if (worker_list_is_running()) {
		logger_t logger = gru_logger_get();

		logger(GRU_INFO, "Stopping test execution because a peer reported a test failure: %s",
			request->payload->notification.body.message);

		if (!worker_manager_abort()) {
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *receiverd_handle_halt(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a halt request");
	receiverd_handle_stop(request, response, pinfo);
	halt = true;
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *receiverd_handle_stats(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Just received a stats request: %s", pinfo->id);

	if (!worker_list_is_running()) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

		return NULL;
	}

	gru_node_t *node = worker_list_root();

	uint64_t total_msg = 0;
	uint64_t total_lat = 0;
	double total_rate = 0.0;

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		total_msg += worker_info->snapshot.count;
		total_lat += gru_time_to_milli(&worker_info->snapshot.latency.elapsed);
		total_rate += worker_info->snapshot.throughput.rate;

		node = node->next;
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_STATS);

	uint32_t count_children = worker_list_count();
	maestro_note_stats_set_child_count(response, count_children);
	logger(GRU_INFO, "Number of childs evaluated: %d", count_children);

	maestro_note_stats_set_role(response, "receiver");
	maestro_note_stats_set_roleinfo(response, "perf");
	maestro_note_stats_set_stat_type(response, MAESTRO_STAT_PERF);

	gru_timestamp_t now = gru_time_now();
	char *formatted_ts = gru_time_write_str(&now);

	maestro_note_stats_set_perf_ts(response, formatted_ts);
	gru_dealloc_string(&formatted_ts);

	maestro_note_stats_set_perf_count(response, total_msg);
	maestro_note_stats_set_perf_rate(response, (total_rate / count_children));
	maestro_note_stats_set_perf_latency(response, ((double) total_lat / count_children));

	return NULL;
}

static maestro_sheet_t *receiverd_new_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new(status);

	if (!ret) {
		return NULL;
	}

	maestro_sheet_set_location(ret, MAESTRO_DAEMON_SHEETS_COUNT, locations, MAESTRO_DEFAULT_QOS);

	maestro_instrument_t *start_instrument =
		maestro_instrument_new(MAESTRO_NOTE_START_RECEIVER, receiverd_handle_start, status);

	maestro_sheet_add_instrument(ret, start_instrument);

	maestro_instrument_t *stop_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STOP_RECEIVER, receiverd_handle_stop, status);

	maestro_sheet_add_instrument(ret, stop_instrument);

	maestro_instrument_t *flush_instrument =
		maestro_instrument_new(MAESTRO_NOTE_FLUSH, commond_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument =
		maestro_instrument_new(MAESTRO_NOTE_SET, receiverd_handle_set, status);

	maestro_sheet_add_instrument(ret, set_instrument);

	maestro_instrument_t *ping_instrument =
		maestro_instrument_new(MAESTRO_NOTE_PING, commond_handle_ping, status);

	maestro_sheet_add_instrument(ret, ping_instrument);

	maestro_instrument_t *stats_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STATS, receiverd_handle_stats, status);

	maestro_sheet_add_instrument(ret, stats_instrument);

	maestro_instrument_t *halt_instrument =
		maestro_instrument_new(MAESTRO_NOTE_HALT, receiverd_handle_halt, status);

	maestro_sheet_add_instrument(ret, halt_instrument);

	maestro_instrument_t *test_fail_notif =
		maestro_instrument_new(MAESTRO_NOTE_NOTIFY_FAIL, receiverd_handle_test_failed, status);

	maestro_sheet_add_instrument(ret, test_fail_notif);

	return ret;
}

static bool receiverd_worker_eval_latency(worker_info_t *worker_info, gru_status_t *status) {
	const int64_t warm_up_secs = 90;

	int64_t elapsed = gru_time_elapsed_secs(worker_info->snapshot.start, worker_info->snapshot.now);
	if (elapsed < warm_up_secs) {
		logger_t logger = gru_logger_get();

		logger(GRU_WARNING, "Current latency (%"PRIi64") exceeds the maximum (%"PRIi64") while still in warm up period");

		return true;
	}


	int64_t latency = gru_time_to_milli(&worker_info->snapshot.latency.elapsed);

	if (latency > worker_options.condition.latency) {
		gru_status_set(status, GRU_FAILURE,
					   "Current latency %"PRIi64" exceeds the maximum value %" PRIi64"",
					   latency, worker_options.condition.latency);

		return false;
	}

	return true;
}


static worker_ret_t receiverd_worker_execute(const vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker.options = &worker_options;

	stats_writer_t writer = {0};
	worker.writer = &writer;
	worker.name = "receiverd";
	worker.worker_flags = WRK_RECEIVER | WRK_DAEMON | WRK_FORKED;
	worker.can_continue = worker_check;

	worker.report_format = FORMAT_HDR;

	worker_handler_t worker_handler = {0};
	worker_ret_t ret;

	char worker_log_dir[PATH_MAX] = {0};
	if (!worker_log_init(worker_log_dir, &status)) {
		return WORKER_FAILURE;
	}
	worker.log_dir = worker_log_dir;


	if (worker.options->rate > 0) {
		logger(GRU_INFO, "Launching rate-based workers for the test");
		worker.naming_options = NM_RATE | NM_LATENCY;
		ret = worker_manager_clone(&worker, rate_receiver_start, &status);

		if (worker.options->condition.latency > 0) {
			worker_handler.flags = WRK_HANDLE_EVAL;
			worker_handler.eval = receiverd_worker_eval_latency;
		}
	}
	else {
		logger(GRU_INFO, "Launching naive workers for the test");
		worker.naming_options = NM_LATENCY | NM_THROUGHPUT;
		ret = worker_manager_clone(&worker, naive_receiver_start, &status);
	}

	if (worker_error(ret) && worker_child(ret)) {
		logger(GRU_ERROR, "Unable to initialize children: %s", status.message);

		return ret;
	}

	if (worker_error(ret)) {
		logger(GRU_ERROR, "Unable to created worker clones: %s", status.message);

		return ret;
	}

	if (worker_child(ret)) {
		return ret;
	}

	worker_manager_watchdog_loop(&worker_handler, &status);
	if (!gru_status_success(&status)) {
		logger(GRU_ERROR, "Test failed: %s", status.message);

		worker_log_link_create(worker_log_dir, options_get_log_dir(), "lastFailed");

		maestro_notify_test_failed(&status);
	}
	else {
		worker_log_link_create(worker_log_dir, options_get_log_dir(), "lastSuccessful");

		maestro_notify_test_successful(&status);
	}

	worker_log_link_create(worker_log_dir, options_get_log_dir(), "last");

	worker.writer->rate.finalize(&status);
	worker.writer->latency.finalize(&status);

	worker_manager_stop();
	return ret;
}

int receiverd_worker_start(const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	worker_ret_t ret = WORKER_SUCCESS;

	maestro_sheet_t *sheet = NULL;
	if (options_get_maestro_host()) {
		sheet = receiverd_new_sheet(&status);
		if (!maestro_player_start(options, sheet, &status)) {
			logger(GRU_FATAL, "Unable to connect to maestro broker: %s", status.message);

			maestro_player_stop(sheet, &status);
			maestro_sheet_destroy(&sheet);
			fflush(NULL);
			return 1;
		}
	}

	while (!halt) {
		usleep(MPT_WORKER_IDLE_TIME);

		if (started) {
			vmsl_t vmsl = vmsl_init();

			if (!vmsl_assign_by_url(&worker_options.uri, &vmsl)) {
				char *uri = gru_uri_simple_format(&worker_options.uri, &status);

				if (!uri) {
					logger(GRU_ERROR, "Unable to assign a VMSL: %s", status.message);
				} else {
					logger(GRU_ERROR, "Unable to assign a VMSL for the URI: %s", uri);
					gru_dealloc_string(&uri);

					break;
				}
			} else {
				ret = receiverd_worker_execute(&vmsl);
				if (ret & WORKER_CHILD) {
					// Child return
					break;
				}

				started = false;

			}
		}

		fflush(NULL);
	}

	if (sheet) {
		if (!(ret & WORKER_CHILD)) {
			maestro_player_stop(sheet, &status);
		}

		maestro_sheet_destroy(&sheet);
	}

	if (worker_success(ret)) {
		return 0;
	}

	return 1;
}