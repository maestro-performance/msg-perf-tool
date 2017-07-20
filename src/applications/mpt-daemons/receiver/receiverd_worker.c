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
#include "receiverd_worker.h"

bool started = false;
bool halt = false;

worker_options_t worker_options = {0};
static worker_list_t *children = NULL;
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

	logger(INFO, "Just received a start request");
	if (started == true || children != NULL) {
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

	logger(INFO, "Just received a stop request");
	started = false;

	if (children) {
		if (!worker_manager_stop(children)) {
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			worker_list_destroy(&children);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *receiverd_handle_test_failed(const maestro_note_t *request,
								   maestro_note_t *response,
								   const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Stopping test execution because a peer reported a test failure");
	started = false;

	if (children) {
		if (!worker_manager_stop(children)) {
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

			worker_list_destroy(&children);

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

	logger(INFO, "Just received a halt request");
	receiverd_handle_stop(request, response, pinfo);
	halt = true;
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *receiverd_handle_stats(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a stats request: %s", pinfo->id);

	if (!worker_list_active(children)) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);

		return NULL;
	}

	gru_node_t *node = worker_list_root(children);

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

	uint32_t count_children = worker_list_count(children);
	maestro_note_stats_set_child_count(response, count_children);
	logger(INFO, "Number of childs evaluated: %d", count_children);

	maestro_note_stats_set_role(response, "receiver");
	maestro_note_stats_set_roleinfo(response, "perf");
	maestro_note_stats_set_stat_type(response, MAESTRO_STAT_PERF);

	gru_timestamp_t now = gru_time_now();
	char *formatted_ts = gru_time_write_str(&now);

	maestro_note_stats_set_perf_ts(response, formatted_ts);
	gru_dealloc_string(&formatted_ts);

	maestro_note_stats_set_perf_count(response, total_msg);
	maestro_note_stats_set_perf_rate(response, (total_rate / count_children));
	maestro_note_stats_set_perf_latency(response, (total_lat / count_children));

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

static bool receiverd_worker_eval_rate(worker_info_t *worker_info, gru_status_t *status) {
	if (worker_info->snapshot.throughput.rate < worker_options.condition.rate) {
		return false;
	}

	return true;
}

static bool receiverd_worker_eval_latency(worker_info_t *worker_info, gru_status_t *status) {
	int64_t latency = gru_time_to_milli(&worker_info->snapshot.latency.elapsed);

	if (latency > worker_options.condition.latency) {
		gru_status_set(status, GRU_FAILURE,
					   "Current latency %"PRIi64" exceeds the maximum value %" PRIi64"",
					   latency, worker_options.condition.latency);

		return false;
	}

	return true;
}

static bool receiverd_worker_execute(const vmsl_t *vmsl) {
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

	worker.report_format = FORMAT_CSV;


	worker_handler_t worker_handler = {0};

	if (worker.options->rate > 0) {
		logger(INFO, "Launching rate-based workers for the test");
		worker.naming_options = NM_RATE | NM_LATENCY;
		children = worker_manager_clone(&worker, rate_receiver_start, &status);

		worker_handler.flags = WRK_HANDLE_EVAL;
		worker_handler.eval = receiverd_worker_eval_latency;
	}
	else {
		logger(INFO, "Launching naive workers for the test");
		worker.naming_options = NM_LATENCY | NM_THROUGHPUT;
		children = worker_manager_clone(&worker, naive_receiver_start, &status);
	}

	if (!children && !gru_status_success(&status)) {
		logger(ERROR, "Unable to initialize children: %s", status.message);

		return true;
	} else {
		// Child return
		if (!children) {
			return false;
		}
	}

	worker_manager_watchdog_loop(children, &worker_handler, &status);
	if (!gru_status_success(&status)) {
		logger(ERROR, "Test failed: %s", status.message);

		maestro_notify_test_failed(&status);
	}

	worker_list_destroy(&children);
	return true;
}

int receiverd_worker_start(const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	bool parent = true;

	maestro_sheet_t *sheet = NULL;
	if (options->maestro_uri.host) {
		sheet = receiverd_new_sheet(&status);
		if (!maestro_player_start(options, sheet, &status)) {
			logger(FATAL, "Unable to connect to maestro broker: %s", status.message);

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
					logger(ERROR, "Unable to assign a VMSL: %s", status.message);
				} else {
					logger(ERROR, "Unable to assign a VMSL for the URI: %s", uri);
					gru_dealloc_string(&uri);
				}
			} else {
				parent = receiverd_worker_execute(&vmsl);
				if (!parent) {
					break;
				}

				started = false;

			}
		}

		fflush(NULL);
	}

	if (sheet) {
		if (parent) {
			maestro_player_stop(sheet, &status);
		}

		maestro_sheet_destroy(&sheet);
	}

	return 0;
}