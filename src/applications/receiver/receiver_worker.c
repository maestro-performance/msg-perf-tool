/**
 Copyright 2016 Otavio Rodolfo Piske

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#include "receiver_worker.h"
#include "vmsl.h"

bool can_start = false;
static char *locations[] = MAESTRO_RECEIVER_DAEMON_SHEETS;

static void *receiver_handle_start(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo) {
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	can_start = true;
	return NULL;
}

static maestro_sheet_t *receiver_new_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new(status);

	if (!ret) {
		return NULL;
	}

	maestro_sheet_set_location(ret, MAESTRO_DAEMON_SHEETS_COUNT, locations, MAESTRO_DEFAULT_QOS);

	maestro_instrument_t *instrument =
		maestro_instrument_new(MAESTRO_NOTE_START_RECEIVER, receiver_handle_start, status);

	maestro_sheet_add_instrument(ret, instrument);

	return ret;
}

static bool receiver_initialize_writer(stats_writer_t *writer,
	const options_t *options,
	gru_status_t *status) {
	if (options->logdir) {
		naming_info_t naming_info = {0};

		naming_info.source = "receiver";

		naming_info.pid = getpid();
		naming_info.ppid = 0;
		naming_info.location = options->logdir;

		return naming_initialize_writer(
			writer, FORMAT_CSV, NM_LATENCY | NM_THROUGHPUT, &naming_info, status);
	} else {
		if (options->parallel_count > 1) {
			return naming_initialize_writer(
				writer, FORMAT_NOP, NM_LATENCY | NM_THROUGHPUT, NULL, status);
		}
	}

	return naming_initialize_writer(
		writer, FORMAT_OUT, NM_LATENCY | NM_THROUGHPUT, NULL, status);
}

static bool receiver_print_partial(worker_info_t *worker_info) {
	worker_snapshot_t snapshot = {0};
	logger_t logger = gru_logger_get();

	if (shr_buff_read(worker_info->shr, &snapshot, sizeof(worker_snapshot_t))) {
		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(INFO,
			"Partial summary: PID %d received %" PRIu64 " messages in %" PRIu64
			" seconds (rate: %.2f msgs/sec)",
			worker_info->child,
			snapshot.count,
			elapsed,
			snapshot.throughput.rate);
	}

	return true;
}

int receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	maestro_sheet_t *sheet = receiver_new_sheet(&status);
	if (!maestro_player_start(options, sheet, &status)) {
		logger(FATAL, "Unable to connect to maestro broker: %s", status.message);

		maestro_player_stop(sheet, &status);
		maestro_sheet_destroy(&sheet);
		fflush(NULL);
		return 1;
	}

	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker_options_t wrk_opt = {0};
	worker.options = &wrk_opt;

	worker.options->uri = options->uri;
	if (options->count == 0) {
		worker.options->duration_type = TEST_TIME;
		worker.options->duration.time = options->duration;
	}
	worker.options->parallel_count = options->parallel_count;
	worker.options->log_level = options->log_level;
	worker.options->message_size = options->message_size;
	worker.options->throttle = options->throttle;
	worker.name = "receiver";

	stats_writer_t writer = {0};
	worker.writer = &writer;
	if (!receiver_initialize_writer(worker.writer, options, &status)) {
		logger(FATAL, "Error initializing performance report writer: %s", status.message);
		return 1;
	}

	worker.can_continue = worker_check;

	if (options->parallel_count == 1) {
		worker.worker_flags = WRK_RECEIVER;
		worker_ret_t ret = {0};
		worker_snapshot_t snapshot = {0};

		ret = naive_receiver_start(&worker, &snapshot, &status);
		if (ret != WORKER_SUCCESS) {
			fprintf(stderr, "Unable to execute worker: %s\n", status.message);

			return 1;
		}

		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(INFO,
			"Summary: received %" PRIu64 " messages in %" PRIu64
			" seconds (rate: %.2f msgs/sec)",
			snapshot.count,
			elapsed,
			snapshot.throughput.rate);
	} else {
		worker.worker_flags = WRK_RECEIVER | WRK_FORKED;
		worker.report_format = FORMAT_CSV;
		worker.naming_options = NM_LATENCY | NM_THROUGHPUT;

		gru_list_t *children =
			worker_manager_clone(&worker, naive_receiver_start, &status);

		if (!children && !gru_status_success(&status)) {
			logger(ERROR, "Unable to initialize children: %s", status.message);

			return 1;
		} else {
			if (!children) {
				return 0;
			}
		}

		worker_manager_watchdog_loop(children, receiver_print_partial);

		gru_list_clean(children, worker_info_destroy_wrapper);
		gru_list_destroy(&children);
	}

	return 0;
}
