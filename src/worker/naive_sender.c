/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "naive_sender.h"

worker_ret_t naive_sender_start(const worker_t *worker,
								worker_snapshot_t *snapshot,
								gru_status_t *status) {
	logger_t logger = gru_logger_get();
	const uint32_t sample_interval = 10;
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0};

	msg_opt_t opt = {0};
	worker_msg_opt_setup(&opt, MSG_DIRECTION_SENDER, worker->options);
	vmslh_handlers_t handlers = vmslh_new(status);

	msg_ctxt_t *msg_ctxt = worker->vmsl->init(opt, &handlers, status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	vmsl_stat_t ret = worker->vmsl->start(msg_ctxt, status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	volatile shr_data_buff_t *shr = worker_shared_buffer_new(worker, status);
	if (!shr) {
		goto err_exit;
	}

	if (!worker->pl_strategy.init(
		&content_storage, worker->options->message_size, status)) {
		goto err_exit;
	}

	snapshot->start = gru_time_now();
	snapshot->now = snapshot->start;
	gru_timestamp_t last_sample_ts = snapshot->start; // Last sampling timestamp

	useconds_t idle_usec = 0;
	if (worker->options->throttle) {
		idle_usec = 1000000 / worker->options->throttle;
	}

	logger(GRU_DEBUG, "Initializing sender loop");
	while (worker->can_continue(worker->options, snapshot)) {
		worker->pl_strategy.load(&content_storage);

		ret = worker->vmsl->send(msg_ctxt, &content_storage, status);
		if (vmsl_stat_error(ret)) {
			logger(GRU_ERROR, "Error sending data: %s", status->message);

			gru_status_reset(status);
			break;
		}

		snapshot->count++;
		snapshot->now = gru_time_now();

		if (gru_time_elapsed_secs(last_sample_ts, snapshot->now) >= sample_interval) {
			uint64_t processed_count = snapshot->count - last_count;

			calc_throughput(
				&snapshot->throughput, last_sample_ts, snapshot->now, processed_count);

			if (unlikely(
				!worker->writer->throughput.write(&snapshot->throughput, status))) {
				logger(GRU_ERROR, "Unable to write throughput data: %s", status->message);

				gru_status_reset(status);
				break;
			}

			last_count = snapshot->count;
			last_sample_ts = snapshot->now;

			shr_buff_write(shr, snapshot, sizeof(worker_snapshot_t));

			fflush(NULL);
		}

		if (worker->options->throttle > 0) {
			usleep(idle_usec);
		}
	}
	logger(GRU_DEBUG, "Finalizing sender loop");

	worker->vmsl->stop(msg_ctxt, status);
	worker->vmsl->destroy(msg_ctxt, status);

	worker->writer->throughput.finalize(status);

	calc_throughput(
		&snapshot->throughput, snapshot->start, snapshot->now, snapshot->count);

	uint64_t elapsed = gru_time_elapsed_secs(snapshot->start, snapshot->now);
	logger(GRU_INFO,
		   "Summary: sent %" PRIu64 " messages in %" PRIu64
			   " seconds (last measured rate: %.2f msgs/sec)",
		   snapshot->count,
		   elapsed,
		   snapshot->throughput.rate);

	shr_buff_detroy(&shr);

	worker->pl_strategy.cleanup(&content_storage);
	worker_msg_opt_cleanup(&opt);
	vmslh_cleanup(&handlers);

	return WORKER_SUCCESS;

	err_exit:
	worker->pl_strategy.cleanup(&content_storage);

	if (msg_ctxt) {
		gru_status_t tmp_status = gru_status_new();

		worker->vmsl->destroy(msg_ctxt, &tmp_status);
	}

	shr_buff_detroy(&shr);

	worker_msg_opt_cleanup(&opt);
	vmslh_cleanup(&handlers);

	return WORKER_FAILURE;
}

