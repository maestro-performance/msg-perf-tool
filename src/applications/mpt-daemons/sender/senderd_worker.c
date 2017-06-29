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
bool halt = false;
worker_options_t worker_options = {0};
static gru_list_t *children = NULL;

static void *senderd_handle_set(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo)
{
	void *ret = commond_handle_set(request, response, &worker_options);
	maestro_note_response_set_id(response, pinfo->id);
	maestro_note_response_set_name(response, pinfo->name);

	return ret;
}

static void *senderd_handle_start(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	if (started == true || children != NULL) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
	} else {
		started = true;

		maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
		maestro_note_response_set_id(response, pinfo->id);
		maestro_note_response_set_name(response, pinfo->name);
	}
	return NULL;
}

static void *senderd_handle_stop(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a stop request");
	started = false;

	if (children) {
		if (!abstract_worker_stop(children)) {
			maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
			maestro_note_response_set_id(response, pinfo->id);
			maestro_note_response_set_name(response, pinfo->name);

			gru_list_clean(children, worker_info_destroy_wrapper);
			gru_list_destroy(&children);

			return NULL;
		}
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);
	maestro_note_response_set_id(response, pinfo->id);
	maestro_note_response_set_name(response, pinfo->name);
	return NULL;
}

static void *senderd_handle_halt(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a halt request");
	senderd_handle_stop(request, response, pinfo);
	halt = true;
	maestro_note_set_cmd(response, MAESTRO_NOTE_OK);

	return NULL;
}

static void *senderd_handle_stats(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a stats request: %s", pinfo->id);

	if (children == NULL) {
		maestro_note_set_cmd(response, MAESTRO_NOTE_INTERNAL_ERROR);
		maestro_note_response_set_id(response, pinfo->id);
		maestro_note_response_set_name(response, pinfo->name);
		return NULL;
	}

	gru_node_t *node = children->root;

	uint64_t total_msg = 0;
	double total_rate = 0.0;

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		total_msg += worker_info->snapshot.count;
		total_rate += worker_info->snapshot.throughput.rate;

		node = node->next;
	}

	maestro_note_set_cmd(response, MAESTRO_NOTE_STATS);
	maestro_note_response_set_id(response, pinfo->id);
	maestro_note_response_set_name(response, pinfo->name);

	uint32_t childs = gru_list_count(children);
	maestro_note_stats_set_child_count(response, childs);
	logger(INFO, "Number of children evaluated: %d", childs);

	maestro_note_stats_set_role(response, "sender");
	maestro_note_stats_set_roleinfo(response, "perf");
	maestro_note_stats_set_stat_type(response, MAESTRO_STAT_PERF);

	gru_timestamp_t now = gru_time_now();
	char *formatted_ts = gru_time_write_str(&now);

	maestro_note_stats_set_perf_ts(response, formatted_ts);
	gru_dealloc_string(&formatted_ts);

	maestro_note_stats_set_perf_count(response, total_msg);
	maestro_note_stats_set_perf_rate(response, (total_rate / childs));
	maestro_note_stats_set_perf_latency(response, 0);

	return NULL;
}

static maestro_sheet_t *new_receiver_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);

	if (!ret) {
		return NULL;
	}

	maestro_instrument_t *start_instrument =
		maestro_instrument_new(MAESTRO_NOTE_START_SENDER, senderd_handle_start, status);

	maestro_sheet_add_instrument(ret, start_instrument);

	maestro_instrument_t *stop_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STOP_SENDER, senderd_handle_stop, status);

	maestro_sheet_add_instrument(ret, stop_instrument);

	maestro_instrument_t *flush_instrument =
		maestro_instrument_new(MAESTRO_NOTE_FLUSH, commond_handle_flush, status);

	maestro_sheet_add_instrument(ret, flush_instrument);

	maestro_instrument_t *set_instrument =
		maestro_instrument_new(MAESTRO_NOTE_SET, senderd_handle_set, status);

	maestro_sheet_add_instrument(ret, set_instrument);

	maestro_instrument_t *ping_instrument =
		maestro_instrument_new(MAESTRO_NOTE_PING, commond_handle_ping, status);

	maestro_sheet_add_instrument(ret, ping_instrument);

	maestro_instrument_t *stats_instrument =
		maestro_instrument_new(MAESTRO_NOTE_STATS, senderd_handle_stats, status);

	maestro_sheet_add_instrument(ret, stats_instrument);

	maestro_instrument_t *halt_instrument =
		maestro_instrument_new(MAESTRO_NOTE_HALT, senderd_handle_halt, status);

	maestro_sheet_add_instrument(ret, halt_instrument);

	return ret;
}

static bool senderd_copy_partial(worker_info_t *worker_info) {
	if (!shr_buff_read(
			worker_info->shr, &worker_info->snapshot, sizeof(worker_snapshot_t))) {
		logger_t logger = gru_logger_get();

		logger(WARNING,
			"Unable to obtain performance snapshot from sender child %d",
			worker_info->child);
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
	worker.worker_flags = WRK_SENDER | WRK_DAEMON | WRK_FORKED;

	worker.can_continue = worker_check;

	worker.report_format = FORMAT_CSV;
	worker.naming_options = NM_LATENCY | NM_THROUGHPUT;

	pl_strategy_assign(&worker.pl_strategy, worker.options->variable_size);

	children = abstract_worker_clone(&worker, abstract_sender_worker_start, &status);

	if (!children && !gru_status_success(&status)) {
		logger(ERROR, "Unable to initialize children: %s", status.message);

		return true;
	} else {
		if (!children) {
			return false;
		}
	}

	while (gru_list_count(children) > 0) {
		mpt_trace("There are still %d children running", gru_list_count(children));
		abstract_worker_watchdog(children, senderd_copy_partial);

		sleep(1);
	}

	gru_list_clean(children, worker_info_destroy_wrapper);
	gru_list_destroy(&children);

	return true;
}

int senderd_worker_start(const options_t *options) {
	gru_status_t status = gru_status_new();
	maestro_sheet_t *sheet = new_receiver_sheet(&status);
	logger_t logger = gru_logger_get();
	bool parent = true;

	if (!maestro_player_start(options, sheet, &status)) {
		logger(FATAL, "Unable to connect to maestro broker: %s", status.message);

		maestro_player_stop(sheet, &status);
		maestro_sheet_destroy(&sheet);
		return 1;
	}

	while (!halt) {
		sleep(1);

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
				parent = senderd_worker_execute(&vmsl);
				if (!parent) {
					// Child return
					break;
				}

				started = false;
			}
		}

		fflush(NULL);
	}

	if (parent) {
		maestro_player_stop(sheet, &status);
	}

	maestro_sheet_destroy(&sheet);

	return 0;
}
