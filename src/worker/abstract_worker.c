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

static const char *worker_name(const worker_t *worker, pid_t child, gru_status_t *status) {
	char *cname = NULL;

	if (asprintf(&cname, "%s-%d", worker->name, child) == -1) {
		gru_status_set(status, GRU_FAILURE, "Not enough memory to format name");

		return NULL;
	}

	return cname;
}

static void abstract_worker_msg_opt(msg_opt_t *opt, msg_direction_t direction, const worker_options_t *options) {
	opt->direction = direction;
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
	abstract_worker_msg_opt(&opt, MSG_DIRECTION_RECEIVER, worker->options);

	msg_ctxt_t *msg_ctxt = worker->vmsl->init(opt, NULL, status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	vmsl_stat_t ret = worker->vmsl->subscribe(msg_ctxt, NULL, status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

#ifdef MPT_SHARED_BUFFERS
	char *cname = worker_name(worker, getpid(), status);
	if (!cname) {
		goto err_exit;
	}

	shr_data_buff_t *shr = shr_buff_new(BUFF_WRITE, sizeof(worker_snapshot_t), 
		cname, status);
	gru_dealloc_string(&cname);

	if (!shr) {
		gru_status_set(status, GRU_FAILURE, "Unable to open a write buffer: %s", 
			status->message);
			
		return EXIT_FAILURE;
	}

#endif // MPT_SHARED_BUFFERS

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

#ifdef MPT_SHARED_BUFFERS
			shr_buff_write(shr, snapshot, sizeof(worker_snapshot_t));
#endif // MPT_SHARED_BUFFERS
			
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


#ifdef MPT_SHARED_BUFFERS
	shr_buff_detroy(&shr);
#endif // MPT_SHARED_BUFFERS

	msg_content_data_release(&content_storage);
	return WORKER_SUCCESS;

err_exit:
#ifdef MPT_SHARED_BUFFERS
	shr_buff_detroy(&shr);
#endif // MPT_SHARED_BUFFERS

	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		worker->vmsl->destroy(msg_ctxt, status);
	}

	gru_status_reset(status);
	return WORKER_FAILURE;
}


static bool worker_init_data(msg_content_data_t *data, size_t size, gru_status_t *status) {
	msg_content_data_init(data, size, status);
	if (!gru_status_success(status)) {
		msg_content_data_release(data);

		return false;
	}

	msg_content_data_fill(data, 'd');
	return true;
}


worker_ret_t abstract_sender_worker_start(const worker_t *worker, worker_snapshot_t *snapshot, 
	gru_status_t *status) 
{
	logger_t logger = gru_logger_get();
	const uint32_t sample_interval = 10;
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0}; 

	msg_opt_t opt = {0};
	abstract_worker_msg_opt(&opt, MSG_DIRECTION_SENDER, worker->options);

	msg_ctxt_t *msg_ctxt = worker->vmsl->init(opt, NULL, status);
	if (!msg_ctxt) {
		goto err_exit;
	}

#ifdef MPT_SHARED_BUFFERS
	char *cname = worker_name(worker, getpid(), status);
	if (!cname) {
		goto err_exit;
	}

	shr_data_buff_t *shr = shr_buff_new(BUFF_WRITE, sizeof(worker_snapshot_t), 
		cname, status);
	gru_dealloc_string(&cname);

	if (!shr) {
		gru_status_set(status, GRU_FAILURE, "Unable to open a write buffer: %s", 
			status->message);
			
		return EXIT_FAILURE;
	}

#endif // MPT_SHARED_BUFFERS

	// TODO: this neeeds to be replaced w/ a content strategy approach
	if (!worker_init_data(&content_storage, worker->options->message_size, status)) {
		goto err_exit;
	}
	

	snapshot->start = gru_time_now();
	snapshot->now = snapshot->start;
	gru_timestamp_t last_sample_ts = snapshot->start; // Last sampling timestamp

	install_interrupt_handler();

	useconds_t idle_usec = 0;
	if (worker->options->throttle) {
		idle_usec = 1000000 / worker->options->throttle;
	}

	
	while (worker->can_continue(worker->options, snapshot)) {
		vmsl_stat_t ret = worker->vmsl->send(msg_ctxt, &content_storage, status);
		if (vmsl_stat_error(ret)) {
			logger(ERROR, "Error sending data: %s\n", status->message);

			gru_status_reset(status);
			break;
		}


		snapshot->count++;
		snapshot->now = gru_time_now();

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

#ifdef MPT_SHARED_BUFFERS
			shr_buff_write(shr, snapshot, sizeof(worker_snapshot_t));
#endif // MPT_SHARED_BUFFERS
		}

		if (worker->options->throttle > 0) {
				usleep(idle_usec);
		}
	}

	worker->vmsl->stop(msg_ctxt, status);
	worker->vmsl->destroy(msg_ctxt, status);

	worker->writer->throughput.finalize(status);

	calc_throughput(&snapshot->throughput, snapshot->start, snapshot->now, 
		snapshot->count);

#ifdef MPT_SHARED_BUFFERS
	shr_buff_detroy(&shr);
#endif // MPT_SHARED_BUFFERS

	msg_content_data_release(&content_storage);
	return WORKER_SUCCESS;

err_exit:
	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		worker->vmsl->destroy(msg_ctxt, status);
	}

#ifdef MPT_SHARED_BUFFERS
	shr_buff_detroy(&shr);
#endif // MPT_SHARED_BUFFERS

	gru_status_reset(status);
	return WORKER_FAILURE;
}

gru_list_t *abstract_worker_clone(const worker_t *worker, abstract_worker_start worker_start, 
	gru_status_t *status) 
{
	gru_list_t *ret = gru_list_new(status);
	if (!ret) {
		return NULL;
	}

	logger_t logger = gru_logger_get();

	logger(INFO, "Creating %d concurrent operations", worker->options->parallel_count);
	for (uint16_t i = 0; i < worker->options->parallel_count; i++) {
		pid_t child = fork();

		if (child == 0) {
			worker_snapshot_t snapshot = {0};

			worker_ret_t ret = worker_start(worker, &snapshot, status);
			if (ret != WORKER_SUCCESS) {
				fprintf(stderr, "Unable to execute worker: %s\n", status->message);

				return NULL;
			}

			return NULL;
		}
		else {
			logger(INFO, "Created child %d", child);

			worker_info_t *worker_info = gru_alloc(sizeof(worker_info_t), status);
			if (!worker_info) {
				break;
			}

			worker_info->child = child;

			char *cname = worker_name(worker, child, status);
			if (!cname) {
				kill(child, SIGKILL);
				break;
			}

			worker_info->shr = shr_buff_new(BUFF_READ, sizeof(worker_snapshot_t), 
				cname, status);

			if (!worker_info->shr) {
				gru_status_set(status, GRU_FAILURE, "Unable to open a read buffer: %s", 
					status->message);
			
				kill(child, SIGKILL);
				break;
			}

			if (!gru_list_append(ret, worker_info)) {
				shr_buff_detroy(&worker_info->shr);
				gru_dealloc((void **) &worker_info);
				kill(child, SIGKILL);

				gru_status_set(status, GRU_FAILURE, "Unable register new worker: %s", 
					status->message);
			
				break;
			}
		}
	}

	return ret;
}