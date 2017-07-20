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

	if (options->logdir) {
		naming_info_t naming_info = {0};

		naming_info.source = "sender";

		naming_info.pid = getpid();
		naming_info.ppid = 0;
		naming_info.location = options->logdir;

		return naming_initialize_writer(
			writer, FORMAT_CSV, NM_THROUGHPUT, &naming_info, status);
	} else {
		if (options->parallel_count > 1) {
			return naming_initialize_writer(
				writer, FORMAT_NOP, NM_THROUGHPUT, NULL, status);
		}
	}

	return naming_initialize_writer(writer, FORMAT_OUT, NM_THROUGHPUT, NULL, status);
}

static bool perf_print_partial(worker_info_t *worker_info) {
	worker_snapshot_t snapshot = {0};
	logger_t logger = gru_logger_get();

	if (shr_buff_read(worker_info->shr, &snapshot, sizeof(worker_snapshot_t))) {
		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(INFO,
			"Partial summary: PID %d sent %" PRIu64 " messages in %" PRIu64
			" seconds (rate: %.2f msgs/sec)",
			worker_info->child,
			snapshot.count,
			elapsed,
			snapshot.throughput.rate);
	}

	return true;
}

int perf_worker_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker_options_t wrk_opt = {0};
	worker.options = &wrk_opt;

	worker.options->uri = options->uri;
	if (options->count == 0) {
		worker.options->duration_type = TEST_TIME;
		worker.options->duration.time = options->duration;
	} else {
		worker.options->duration_type = MESSAGE_COUNT;
		worker.options->duration.count = options->count;
	}
	worker.options->parallel_count = options->parallel_count;
	worker.options->log_level = options->log_level;
	worker.options->message_size = options->message_size;
	worker.options->throttle = options->throttle;
	worker.name = "sender";

	stats_writer_t writer = {0};
	worker.writer = &writer;
	if (!perf_initialize_writer(worker.writer, options, &status)) {
		logger(FATAL, "Error initializing performance report writer: %s", status.message);
		return 1;
	}
	pl_strategy_assign(&worker.pl_strategy, options->variable_size);

	worker.can_continue = worker_check;

	if (options->parallel_count == 1) {
		worker.worker_flags = WRK_SENDER;

		worker_ret_t ret = {0};
		worker_snapshot_t snapshot = {0};

		worker_wait_setup();
		ret = naive_sender_start(&worker, &snapshot, &status);
		if (ret != WORKER_SUCCESS) {
			logger(ERROR, "Unable to execute worker: %s\n", status.message);

			return 1;
		}

		uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

		logger(INFO,
			"Summary: sent %" PRIu64 " messages in %" PRIu64
			" seconds (rate: %.2f msgs/sec)",
			snapshot.count,
			elapsed,
			snapshot.throughput.rate);
	} else {
		worker.worker_flags = WRK_SENDER | WRK_FORKED;
		worker_list_t *children =
			worker_manager_clone(&worker, naive_sender_start, &status);

		if (!children && !gru_status_success(&status)) {
			logger(ERROR, "Unable to initialize children: %s", status.message);

			return 1;
		} else {
			if (!children) {
				return 0;
			}
		}

		worker_handler_t worker_handler = {0};
		worker_handler.flags = WRK_HANDLE_PRINT;
		worker_handler.print = perf_print_partial;


		worker_manager_watchdog_loop(children, &worker_handler, &status);

		worker_list_destroy(&children);
	}

	return 0;
}