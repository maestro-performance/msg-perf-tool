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
#include "worker_manager.h"

uint32_t errors = 0;

worker_ret_t worker_manager_clone(worker_t *worker,
								 worker_start_fn worker_start,
								 gru_status_t *status) {
	errors = 0;

	if (!worker_list_init(status)) {
		return WORKER_FAILURE;
	}

	logger_t logger = gru_logger_get();

	if (!worker_dump(worker->log_dir, worker->options, status)) {
		return WORKER_FAILURE;
	}

	naming_info_t naming_info = {0};

	naming_info.source = worker->name;
	naming_info.location = worker->log_dir;

	bool nmret = naming_initialize_writer(worker->writer, worker->report_format,
										  worker->naming_options, &naming_info, status);
	if (!nmret) {
		logger(GRU_ERROR, "Error initializing performance report writer: %s",
			   status->message);
		return WORKER_FAILURE;
	}

	logger(GRU_INFO, "Creating %d concurrent operations", worker->options->parallel_count);
	for (uint16_t i = 0; i < worker->options->parallel_count; i++) {
		pid_t child = fork();

		if (child == 0) {
			worker_snapshot_t snapshot = {0};

			remap_log(worker->log_dir, worker->name, getppid(), getpid(), stderr, status);

			install_interrupt_handler();
			logger(GRU_TRACE, "Starting worker %d", i);
			worker_ret_t wret = worker_start(worker, &snapshot, status);
			if (wret != WORKER_SUCCESS) {
				logger(GRU_ERROR, "Unable to execute worker: %s", status->message);
			}
			else {
				logger(GRU_INFO, "Test execution terminated");
			}

			return wret | WORKER_CHILD;
		}

		worker_info_t *worker_info = worker_info_new(worker, child, status);
		if (!worker_info) {
			logger(GRU_ERROR, "Unable to create worker info: %s", status->message);

			return WORKER_FAILURE;
		}

		if (!worker_list_append(worker_info, status)) {
			kill(worker_info->child, SIGKILL);
			worker_info_destroy(&worker_info);

			return WORKER_FAILURE;
		}
	}

	return WORKER_SUCCESS;
}

static bool worker_manager_update_snapshot(worker_info_t *worker_info) {


	gru_status_t status = gru_status_new();
	logger_t logger = gru_logger_get();

	worker_queue_stat_t queue_stat;
	queue_stat = worker_queue_read(worker_info->pqueue, &worker_info->snapshot,
								   sizeof(worker_snapshot_t));
	while (wq_has_data(queue_stat)) {
		if (worker_info->worker_flags & WRK_RECEIVER) {
			if (unlikely(!worker_info->writer->latency.write(&worker_info->snapshot.latency, &status))) {

				logger(GRU_ERROR, "Unable to write latency data: %s", status.message);
			}
		}

		if (unlikely(!worker_info->writer->rate.write(&worker_info->snapshot.throughput,
													  &worker_info->snapshot.eta,
													  &status))) {
			logger(GRU_ERROR, "Unable to write throughput data: %s", status.message);

		}

		queue_stat = worker_queue_read(worker_info->pqueue, &worker_info->snapshot,
									   sizeof(worker_snapshot_t));
	};

	return true;
}

static bool worker_manager_watchdog(worker_handler_t *handler, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	worker_list_lock();
	gru_node_t *node = worker_list_root();

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		int wstatus = 0;
		pid_t pid = waitpid(worker_info->child, &wstatus, WNOHANG);

		// waitpid returns 0 if WNOHANG and there's no change of state for the process
		if (pid == 0) {
			worker_manager_update_snapshot(worker_info);

			if (handler->flags & WRK_HANDLE_PRINT) {
				if (!handler->print) {
					gru_status_set(status, GRU_FAILURE,
								   "Worker handler print flag is set but no print function given");

					return false;
				}

				handler->print(worker_info);
			}

			if (handler->flags & WRK_HANDLE_EVAL) {
				if (!handler->eval) {
					gru_status_set(status, GRU_FAILURE,
								   "Worker handler eval flag is set but no eval function given");

					return false;
				}

				if (!handler->eval(worker_info, status)) {
					logger(GRU_DEBUG, "Worker handler eval failed: %s", status->message);

					worker_list_unlock();
					return false;
				}
			}

			node = node->next;
		} else {
			if (WIFEXITED(wstatus)) {
				logger(GRU_INFO,
					   "Child %d finished with status %d",
					   worker_info->child,
					   WEXITSTATUS(wstatus));
			} else if (WIFSIGNALED(wstatus)) {
				int sig = WTERMSIG(wstatus);

				if (sig != SIGINT) {
					errors++;
					logger(GRU_INFO,
						   "Child %d received a signal %d",
						   worker_info->child,
						   WTERMSIG(wstatus));
				}
			} else if (WIFSTOPPED(wstatus)) {
				int sig = WSTOPSIG(wstatus);

				if (sig != SIGINT) {
					errors++;

					logger(GRU_ERROR, "Child %d stopped %d", worker_info->child, sig);
				}
			}


			node = worker_list_remove_unlocked(node);
			worker_info_destroy(&worker_info);
		}
	}

	worker_list_unlock();
	return true;
}

void worker_manager_watchdog_loop(worker_handler_t *handler, gru_status_t *status) {
	uint32_t count = worker_list_count();

	int wait_time = 100000;

	if (count > 10) {
		wait_time = 1000000 / count;
	}


	while (worker_list_is_running() && count > 0) {
		mpt_trace("There are still %d children running", count);
		if (worker_manager_watchdog(handler, status)) {
			usleep(wait_time);
			count = worker_list_count();
		} else {
			break;
		}
	}

	if (errors > 0) {
		gru_status_set(status,  GRU_FAILURE, "There were %"PRIu32" child process that ended with failure", errors);
	}
}

static bool worker_manager_do_stop(int signal) {

	logger_t logger = gru_logger_get();

	worker_list_lock();
	gru_node_t *node = worker_list_root();

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		logger(GRU_INFO, "%s child %d", (signal == SIGTERM ? "Aborting" : "Stopping"), worker_info->child);
		if (kill(worker_info->child, signal) != 0) {
			logger(GRU_WARNING, "Unable to send signal to the child process");
			node = node->next;
			continue;
		}

		uint16_t retry = 10;
		pid_t pid = 0;

		do {
			int wstatus = 0;

			pid = waitpid(worker_info->child, &wstatus, WNOHANG);

			// waitpid returns 0 if WNOHANG and there's no change of state for the process
			// otherwise it returns the pid of the process
			if (pid > 0) {
				if (WIFEXITED(wstatus)) {
					logger(GRU_INFO,
						   "Child %d stopped successfully with status %d",
						   worker_info->child,
						   WEXITSTATUS(wstatus));
				} else if (WIFSIGNALED(wstatus)) {
					logger(GRU_INFO,
						   "Child %d stopped successfully signal %d",
						   worker_info->child,
						   WTERMSIG(wstatus));
				} else if (WIFSTOPPED(wstatus)) {
					logger(GRU_WARNING,
						   "Child %d stopped abnormally %d",
						   worker_info->child,
						   WSTOPSIG(wstatus));
				}

				break;
			}

			if (pid == -1) {
				logger(GRU_WARNING,
					   "Failed to stop child %d: %s",
					   worker_info->child,
					   strerror(errno));

				break;
			}

			usleep(MPT_MANAGER_SHUTDOWN_WAIT);
			retry--;

			if (retry == 0) {
				logger(GRU_WARNING,
					   "Killing child process %d because refuses to stop for good",
					   worker_info->child);
				kill(worker_info->child, SIGKILL);
			}

		} while (pid == 0 && retry > 0);

		node = node->next;
	}

	worker_list_unlock();
	worker_list_clean();
	return true;
}


bool worker_manager_stop() {
	// Use sigint for graceful shutdown
	return worker_manager_do_stop(SIGINT);
}


bool worker_manager_abort() {
	errors++;
	return worker_manager_do_stop(SIGTERM);
}