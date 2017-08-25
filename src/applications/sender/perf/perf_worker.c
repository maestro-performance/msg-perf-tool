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
#include "perf_worker.h"

static bool perf_initialize_writer(stats_writer_t *writer,
	const options_t *options,
	gru_status_t *status) {

	if (options_get_log_dir()) {
		naming_info_t naming_info = {0};

		naming_info.source = "sender";

		naming_info.location = options_get_log_dir();

		return naming_initialize_writer(
			writer, FORMAT_CSV, NM_THROUGHPUT, &naming_info, status);
	} else {
		if (options_get_parallel_count() > 1) {
			return naming_initialize_writer(
				writer, FORMAT_NOP, NM_THROUGHPUT, NULL, status);
		}
	}

	return naming_initialize_writer(writer, FORMAT_OUT, NM_THROUGHPUT, NULL, status);
}

static bool perf_print_partial(worker_info_t *worker_info) {
	worker_snapshot_t snapshot = {0};
	logger_t logger = gru_logger_get();

	uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

	logger(GRU_INFO,
		"Partial summary: PID %d sent %" PRIu64 " messages in %" PRIu64
		" seconds (rate: %.2f msgs/sec)",
		worker_info->child,
		snapshot.count,
		elapsed,
		snapshot.throughput.rate);

	return true;
}

int perf_worker_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker_options_t wrk_opt = {0};
	worker.options = &wrk_opt;

	worker.options->uri = options_get_broker_uri();
	if (options_get_count() == 0) {
		worker.options->duration_type = TEST_TIME;
		worker.options->duration.time = options_get_duration();
	} else {
		worker.options->duration_type = MESSAGE_COUNT;
		worker.options->duration.count = options_get_count();
	}
	worker.options->parallel_count = options_get_parallel_count();
	worker.options->log_level = options_get_log_level();
	worker.options->message_size = options_get_message_size();
	worker.options->throttle = options_get_throttle();

	worker.name = "sender";

	char worker_log_dir[PATH_MAX] = {0};
	if (!worker_log_init(worker_log_dir, &status)) {
		return WORKER_FAILURE;
	}
	worker.log_dir = worker_log_dir;

	stats_writer_t writer = {0};
	worker.writer = &writer;
	if (!perf_initialize_writer(worker.writer, options, &status)) {
		logger(GRU_FATAL, "Error initializing performance report writer: %s", status.message);
		return 1;
	}
	pl_strategy_assign(&worker.pl_strategy, options_get_variable_size());

	worker.can_continue = worker_check;

	if (options_get_parallel_count() == 1) {
		worker.worker_flags = WRK_SENDER;

		worker_snapshot_t snapshot = {0};

		worker_ret_t ret = naive_sender_start(&worker, &snapshot, &status);
		if (ret != WORKER_SUCCESS) {
			logger(GRU_ERROR, "Unable to execute worker: %s\n", status.message);

			return 1;
		}

		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(GRU_INFO,
			"Summary: sent %" PRIu64 " messages in %" PRIu64
			" seconds (rate: %.2f msgs/sec)",
			snapshot.count,
			elapsed,
			snapshot.throughput.rate);
	} else {
		worker.worker_flags = WRK_SENDER | WRK_FORKED;
		worker_ret_t ret = worker_manager_clone(&worker, naive_sender_start, &status);

		if (worker_error(ret)) {
			logger(GRU_ERROR, "Unable to initialize children: %s", status.message);

			return 1;
		}

		if (ret & WORKER_CHILD) {
			return 0;
		}

		worker_handler_t worker_handler = {0};
		worker_handler.flags = WRK_HANDLE_PRINT;
		worker_handler.print = perf_print_partial;


		worker_manager_watchdog_loop(&worker_handler, &status);
		worker_manager_stop();
	}

	return 0;
}