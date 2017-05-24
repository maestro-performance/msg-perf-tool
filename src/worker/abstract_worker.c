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

static void abstract_worker_msg_opt(msg_opt_t *opt,
	msg_direction_t direction,
	const worker_options_t *options) {
	opt->direction = direction;
	opt->qos = MSG_QOS_AT_MOST_ONCE;
	opt->statistics = MSG_STAT_DEFAULT;

	msg_conn_info_gen_id(&opt->conn_info);
	opt->uri = options->uri;
}

static void abstract_worker_msg_opt_cleanup(msg_opt_t *opt) {
	msg_conn_info_cleanup(&opt->conn_info);
}


worker_ret_t abstract_receiver_worker_start(const worker_t *worker,
	worker_snapshot_t *snapshot,
	gru_status_t *status) {
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

	volatile shr_data_buff_t *shr = worker_shared_buffer_new(worker, status);
	if (!shr) {
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

	logger(DEBUG, "Initializing receiver loop");
	while (worker->can_continue(worker->options, snapshot)) {
		vmsl_stat_t rstat = worker->vmsl->receive(msg_ctxt, &content_storage, status);
		if (unlikely(vmsl_stat_error(rstat))) {
			if (rstat & VMSL_NO_TIMESTAMP) {
				continue;
			}

			logger(ERROR, "Error receiving data: %s", status->message);

			gru_status_reset(status);
			break;
		}

		snapshot->now = gru_time_now();
		if (rstat & VMSL_NO_DATA) {
			/*
			 * After some measurements, it turns out that 25ms seem to provide the right
			 * balance between responsiveness and throughput.
			 */
			usleep(25000);
			continue;
		}

		snapshot->count++;

		calc_latency(&snapshot->latency, content_storage.created, snapshot->now);
		if (unlikely(!worker->writer->latency.write(&snapshot->latency, status))) {
			logger(ERROR, "Unable to write latency data: %s", status->message);

			gru_status_reset(status);
			break;
		}

		if (gru_time_elapsed_secs(last_sample_ts, snapshot->now) >= sample_interval) {
			uint64_t processed_count = snapshot->count - last_count;

			calc_throughput(
				&snapshot->throughput, last_sample_ts, snapshot->now, processed_count);

			if (unlikely(
					!worker->writer->throughput.write(&snapshot->throughput, status))) {
				logger(ERROR, "Unable to write throughput data: %s", status->message);

				gru_status_reset(status);
				break;
			}

			shr_buff_write(shr, snapshot, sizeof(worker_snapshot_t));

			last_count = snapshot->count;
			last_sample_ts = snapshot->now;
			fflush(NULL);
		}
	}
	logger(DEBUG, "Finalizing sender loop");

	worker->vmsl->stop(msg_ctxt, status);
	worker->vmsl->destroy(msg_ctxt, status);

	worker->writer->latency.finalize(status);
	worker->writer->throughput.finalize(status);

	calc_throughput(
		&snapshot->throughput, snapshot->start, snapshot->now, snapshot->count);

	uint64_t elapsed = gru_time_elapsed_secs(snapshot->start, snapshot->now);

	logger(INFO,
		"Summary: received %" PRIu64 " messages in %" PRIu64
		" seconds (last measured rate: %.2f msgs/sec)",
		snapshot->count,
		elapsed,
		snapshot->throughput.rate);

	shr_buff_detroy(&shr);

	msg_content_data_release(&content_storage);
	abstract_worker_msg_opt_cleanup(&opt);
	return WORKER_SUCCESS;

err_exit:
	shr_buff_detroy(&shr);

	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		worker->vmsl->destroy(msg_ctxt, status);
	}
	abstract_worker_msg_opt_cleanup(&opt);

	gru_status_reset(status);
	return WORKER_FAILURE;
}

worker_ret_t abstract_sender_worker_start(const worker_t *worker,
	worker_snapshot_t *snapshot,
	gru_status_t *status) {
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

	logger(DEBUG, "Initializing sender loop");
	while (worker->can_continue(worker->options, snapshot)) {
		worker->pl_strategy.load(&content_storage);

		vmsl_stat_t ret = worker->vmsl->send(msg_ctxt, &content_storage, status);
		if (vmsl_stat_error(ret)) {
			logger(ERROR, "Error sending data: %s", status->message);

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
				logger(ERROR, "Unable to write throughput data: %s", status->message);

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
	logger(DEBUG, "Finalizing sender loop");

	worker->vmsl->stop(msg_ctxt, status);
	worker->vmsl->destroy(msg_ctxt, status);

	worker->writer->throughput.finalize(status);

	calc_throughput(
		&snapshot->throughput, snapshot->start, snapshot->now, snapshot->count);

	uint64_t elapsed = gru_time_elapsed_secs(snapshot->start, snapshot->now);
	logger(INFO,
		"Summary: sent %" PRIu64 " messages in %" PRIu64
		" seconds (last measured rate: %.2f msgs/sec)",
		snapshot->count,
		elapsed,
		snapshot->throughput.rate);

	shr_buff_detroy(&shr);

	worker->pl_strategy.cleanup(&content_storage);
	abstract_worker_msg_opt_cleanup(&opt);
	return WORKER_SUCCESS;

err_exit:
	worker->pl_strategy.cleanup(&content_storage);

	if (msg_ctxt) {
		worker->vmsl->destroy(msg_ctxt, status);
	}

	shr_buff_detroy(&shr);

	abstract_worker_msg_opt_cleanup(&opt);

	gru_status_reset(status);
	return WORKER_FAILURE;
}

gru_list_t *abstract_worker_clone(worker_t *worker,
	abstract_worker_start worker_start,
	gru_status_t *status) {
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

			const options_t *options = get_options_object();

			remap_log(options->logdir, worker->name, getppid(), getpid(), stderr, status);

			naming_info_t naming_info = {0};

			naming_info.source = worker->name;
			naming_info.location = options->logdir;
			naming_info.pid = getpid();
			naming_info.ppid = getppid();

			bool nmret = naming_initialize_writer(worker->writer,
				worker->report_format,
				worker->naming_options,
				&naming_info,
				status);
			if (!nmret) {
				logger(ERROR, "Error initializing performance report writer: %s",
					 status->message);
				return NULL;
			}

			install_interrupt_handler();
			worker_ret_t wret = worker_start(worker, &snapshot, status);
			if (wret != WORKER_SUCCESS) {
				logger(ERROR, "Unable to execute worker: %s", status->message);
			}

			logger(INFO, "Test execution terminated");
			return NULL;
		} else {
			worker_wait_setup();

			worker_info_t *worker_info = worker_info_new(worker, child, status);
			if (!worker_info) {
				logger(ERROR, "Unable to create worker info: %s", status->message);
				break;
			}

			// worker_info_t *worker_info = gru_alloc(sizeof(worker_info_t), status);
			// if (!worker_info) {
			// 	break;
			// }

			// worker_info->child = child;

			// const char *cname = worker_name(worker, child, status);
			// if (!cname) {
			// 	kill(child, SIGKILL);
			// 	break;
			// }

			// logger(INFO, "Created child %d and waiting for the continue signal", child);
			// worker_wait();

			// logger(INFO, "Child %d gave the ok signal", child);
			// fflush(NULL);

			// worker_info->shr =
			// 	shr_buff_new(BUFF_WRITE, sizeof(worker_snapshot_t), cname, status);
			// gru_dealloc_const_string(&cname);

			// if (!worker_info->shr) {
			// 	gru_status_set(status,
			// 		GRU_FAILURE,
			// 		"Unable to open a read buffer: %s",
			// 		status->message);

			// 	kill(child, SIGKILL);
			// 	break;
			// }

			if (!gru_list_append(ret, worker_info)) {
				shr_buff_detroy(&worker_info->shr);
				gru_dealloc((void **) &worker_info);
				kill(child, SIGKILL);

				gru_status_set(status,
					GRU_FAILURE,
					"Unable register new worker: %s",
					status->message);

				break;
			}
		}
	}

	return ret;
}

bool abstract_worker_watchdog(gru_list_t *list,
	abstract_worker_watchdog_handler handler) {
	gru_node_t *node = NULL;
	logger_t logger = gru_logger_get();

	if (list == NULL) {
		return true;
	}

	node = list->root;

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		int wstatus = 0;
		pid_t pid = waitpid(worker_info->child, &wstatus, WNOHANG);

		// waitpid returns 0 if WNOHANG and there's no change of state for the process
		if (pid == 0) {
			if (handler(worker_info)) {
				node = node->next;
			} else {
				return false;
			}
		} else {
			if (WIFEXITED(wstatus)) {
				logger(INFO,
					"Child %d finished with status %d",
					worker_info->child,
					WEXITSTATUS(wstatus));
			} else if (WIFSIGNALED(wstatus)) {
				logger(INFO,
					"Child %d received a signal %d",
					worker_info->child,
					WTERMSIG(wstatus));
			} else if (WIFSTOPPED(wstatus)) {
				logger(
					ERROR, "Child %d stopped %d", worker_info->child, WSTOPSIG(wstatus));
			}

			gru_node_t *orphan = node;
			node = node->next;

			if (!gru_list_remove_node(list, orphan)) {
				logger(ERROR, "Unable to remove an orphaned child");
			}

			gru_node_destroy(&orphan);
			gru_dealloc((void **) &worker_info);
		}
	}

	return true;
}

bool abstract_worker_stop(gru_list_t *list) {
	gru_node_t *node = NULL;
	logger_t logger = gru_logger_get();

	if (list == NULL) {
		return false;
	}

	node = list->root;

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		logger(INFO, "Terminating child %d", worker_info->child);
		if (kill(worker_info->child, SIGTERM) != 0) {
			logger(WARNING, "Unable to send signal to the child process");
			node = node->next;
			continue;
		}

		uint16_t retry = 3;
		pid_t pid = 0;

		do {
			int wstatus = 0;

			pid = waitpid(worker_info->child, &wstatus, WNOHANG);

			// waitpid returns 0 if WNOHANG and there's no change of state for the process
			if (pid != 0) {
				if (WIFEXITED(wstatus)) {
					logger(INFO,
						"Child %d stopped successfully with status %d",
						worker_info->child,
						WEXITSTATUS(wstatus));
				} else if (WIFSIGNALED(wstatus)) {
					logger(INFO,
						"Child %d stopped successfully signal %d",
						worker_info->child,
						WTERMSIG(wstatus));
				} else if (WIFSTOPPED(wstatus)) {
					logger(WARNING,
						"Child %d stopped abnormally %d",
						worker_info->child,
						WSTOPSIG(wstatus));
				}

				break;
			} else {
				if (pid == -1) {
					logger(WARNING,
						"Failed to stop child %d: %s",
						worker_info->child,
						strerror(errno));

					break;
				} else {
					usleep(10000);
					retry--;

					if (retry == 0) {
						logger(WARNING,
							"Killing child process %d because refuses to stop for good",
							worker_info->child);
						kill(worker_info->child, SIGKILL);
					}
				}
			}
		} while (pid == 0 && retry > 0);

		node = node->next;
	}

	return true;
}
