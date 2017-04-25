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
#include "abstract_worker.h"

static void abstract_worker_msg_opt(msg_opt_t *opt, const worker_options_t *options) {
	opt->direction = MSG_DIRECTION_RECEIVER;
	opt->qos = MSG_QOS_AT_MOST_ONCE;
	opt->statistics = MSG_STAT_DEFAULT;

	msg_conn_info_gen_id(&opt->conn_info);
	opt->uri = options->uri;
}

worker_ret_t abstract_receiver_worker_start(const worker_t *worker, worker_snapshot_t *snapshot, 
	gru_status_t *status) 
{
	logger_t logger = gru_logger_get();
	const uint32_t sample_interval = 10;
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0}; 

	msg_opt_t opt = {0};
	abstract_worker_msg_opt(&opt, worker->options);

	msg_ctxt_t *msg_ctxt = worker->vmsl->init(opt, NULL, status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	vmsl_stat_t ret = worker->vmsl->subscribe(msg_ctxt, NULL, status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	// TODO: requires a content strategy
	msg_content_data_init(&content_storage, worker->options->message_size, status);
	if (!gru_status_success(status)) {
	 	goto err_exit;
	}

	snapshot->start = gru_time_now();
	snapshot->now = snapshot->start;
	gru_timestamp_t last_sample_ts = snapshot->start; // Last sampling timestamp

	install_interrupt_handler();
	
	while (worker->can_continue(worker->options, snapshot)) {
		vmsl_stat_t rstat = worker->vmsl->receive(msg_ctxt, &content_storage, status);
		if (unlikely(vmsl_stat_error(rstat))) {
			logger(ERROR, "Error receiving data: %s\n", status->message);

			gru_status_reset(status);
			break;
		}

		if (rstat & VMSL_NO_DATA) {
			usleep(500);
			continue;
		}

		snapshot->count++;
		snapshot->now = gru_time_now();

		calc_latency(&snapshot->latency, content_storage.created, snapshot->now);
		if (unlikely(!worker->writer->latency.write(&snapshot->latency, status))) {
			logger(ERROR, "Unable to write latency data: %s", status->message);

			gru_status_reset(status);
			break;
		}

		if (gru_time_elapsed_secs(last_sample_ts, snapshot->now) >= sample_interval) {
			uint64_t processed_count = snapshot->count - last_count;
			
			calc_throughput(&snapshot->throughput, last_sample_ts, snapshot->now, processed_count);

			if (unlikely(!worker->writer->throughput.write(&snapshot->throughput, status))) {
				logger(ERROR, "Unable to write throughput data: %s", status->message);

				gru_status_reset(status);
				break;
			} 
			
		 	last_count = snapshot->count;
			last_sample_ts = snapshot->now;
		}
	}

	worker->vmsl->stop(msg_ctxt, status);
	worker->vmsl->destroy(msg_ctxt, status);

	worker->writer->latency.finalize(status);
	worker->writer->throughput.finalize(status);

	calc_throughput(&snapshot->throughput, snapshot->start, snapshot->now, 
		snapshot->count);

	msg_content_data_release(&content_storage);
	return WORKER_SUCCESS;

err_exit:
	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		worker->vmsl->destroy(msg_ctxt, status);
	}

	gru_status_reset(status);
	return WORKER_FAILURE;
}
