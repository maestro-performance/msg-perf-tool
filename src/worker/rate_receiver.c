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
#include "rate_receiver.h"

worker_ret_t rate_receiver_start(const worker_t *worker,
								  worker_snapshot_t *snapshot,
								  gru_status_t *status) {
	logger_t logger = gru_logger_get();
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0};

	msg_opt_t opt = {0};
	worker_msg_opt_setup(&opt, MSG_DIRECTION_RECEIVER, worker->options);

	vmslh_handlers_t handlers = vmslh_new(status);

	mpt_trace("Initializing VMSL");
	msg_ctxt_t *msg_ctxt = worker->vmsl->init(opt, &handlers, status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	mpt_trace("Starting VMSL");
	vmsl_stat_t ret = worker->vmsl->start(msg_ctxt, status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	mpt_trace("Subscribing VMSL");
	ret = worker->vmsl->subscribe(msg_ctxt, NULL, status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	mpt_trace("Creating queue");
	worker_queue_t *pqueue = worker_create_queue(worker, status);
	if (!pqueue) {
		goto err_exit;
	}

	mpt_trace("Initializing content generator");
	msg_content_data_init(&content_storage, worker->options->message_size, status);
	if (!gru_status_success(status)) {
		goto err_exit;
	}

	mpt_trace("Setting up initial time");
	snapshot->start = gru_time_now();
	snapshot->now = snapshot->start;
	snapshot->eta = snapshot->start;
	gru_timestamp_t last_sample_ts = snapshot->start; // Last sampling timestamp

	useconds_t interval = 0;
	if (worker->options->rate) {
		interval = 1000000 / worker->options->rate;
	}

	logger(GRU_DEBUG, "Initializing receiver loop");
	while (worker->can_continue(worker->options, snapshot)) {
		vmsl_stat_t rstat = worker->vmsl->receive(msg_ctxt, &content_storage, status);
		if (unlikely(vmsl_stat_error(rstat))) {
			if (rstat & VMSL_NO_TIMESTAMP) {
				continue;
			}

			logger(GRU_ERROR, "Error receiving data: %s", status->message);

			gru_status_reset(status);
			break;
		}

		snapshot->now = gru_time_now();
		if (rstat & VMSL_NO_DATA) {

			usleep(WORKER_NO_DATA_WAIT);
			continue;
		}

		snapshot->count++;

		calc_latency(&snapshot->latency, content_storage.created, snapshot->now);

		uint64_t processed_count = snapshot->count - last_count;

		calc_throughput(&snapshot->throughput, last_sample_ts, snapshot->now, processed_count);

		worker_queue_write(pqueue, snapshot, sizeof(worker_snapshot_t), NULL);

		last_count = snapshot->count;
		last_sample_ts = snapshot->now;

		gru_time_add_microseconds(&snapshot->eta, interval);
	}
	logger(GRU_DEBUG, "Finalizing sender loop");

	worker->vmsl->stop(msg_ctxt, status);
	worker->vmsl->destroy(msg_ctxt, status);

	calc_throughput(
		&snapshot->throughput, snapshot->start, snapshot->now, snapshot->count);

	uint64_t elapsed = gru_time_elapsed_secs(snapshot->start, snapshot->now);

	logger(GRU_INFO,
		   "Summary: received %" PRIu64 " messages in %" PRIu64
	" seconds (last measured rate: %.2f msgs/sec)",
		snapshot->count,
		elapsed,
		snapshot->throughput.rate);

	worker_queue_destroy(&pqueue);

	msg_content_data_release(&content_storage);
	worker_msg_opt_cleanup(&opt);
	vmslh_cleanup(&handlers);
	return WORKER_SUCCESS;

	err_exit:

	worker_queue_destroy(&pqueue);

	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		gru_status_t tmp_status = gru_status_new();

		worker->vmsl->destroy(msg_ctxt, &tmp_status);
	}
	worker_msg_opt_cleanup(&opt);

	vmslh_cleanup(&handlers);
	return WORKER_FAILURE;
}