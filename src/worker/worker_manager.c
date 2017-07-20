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

bool worker_manager_clone(worker_t *worker,
								 worker_start_fn worker_start,
								 gru_status_t *status) {
	if (!worker_list_init(status)) {
		return false;
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
				return false;
			}

			install_interrupt_handler();
			worker_ret_t wret = worker_start(worker, &snapshot, status);
			if (wret != WORKER_SUCCESS) {
				logger(ERROR, "Unable to execute worker: %s", status->message);
			}

			logger(INFO, "Test execution terminated");
			return false;
		} else {
			worker_wait_setup();

			worker_info_t *worker_info = worker_info_new(worker, child, status);
			if (!worker_info) {
				logger(ERROR, "Unable to create worker info: %s", status->message);
				break;
			}

			if (!worker_list_append(worker_info, status)) {
				kill(worker_info->child, SIGKILL);
				worker_info_destroy(&worker_info);

				break;
			}
		}
	}

	return true;
}

static bool worker_manager_update_snapshot(worker_info_t *worker_info) {
	bool ret;

	ret = shr_buff_read(worker_info->shr, &worker_info->snapshot,
						sizeof(worker_snapshot_t));

	if (!ret) {
		logger_t logger = gru_logger_get();

		logger(WARNING,
			   "Unable to obtain performance snapshot from worker %d",
			   worker_info->child);
	}

	return ret;
}

static bool worker_manager_watchdog(worker_handler_t *handler, gru_status_t *status) {
	gru_node_t *node = NULL;
	logger_t logger = gru_logger_get();

	node = worker_list_root();

	while (node) {
		worker_info_t *worker_info = gru_node_get_data_ptr(worker_info_t, node);

		int wstatus = 0;
		pid_t pid = waitpid(worker_info->child, &wstatus, WNOHANG);

		// waitpid returns 0 if WNOHANG and there's no change of state for the process
		if (pid == 0) {
			worker_manager_update_snapshot(worker_info);

			if (handler->flags & WRK_HANDLE_PRINT) {
				handler->print(worker_info);
			}

			if (handler->flags & WRK_HANDLE_EVAL) {
				if (!handler->eval(worker_info, status)) {
					logger(DEBUG, "Worker handler eval failed: %s", status->message);
					return false;
				}
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


			node = worker_list_remove(node);
			worker_info_destroy(&worker_info);
		}
	}

	return true;
}

void worker_manager_watchdog_loop(worker_handler_t *handler, gru_status_t *status) {
	const int wait_time = 250000;

	uint32_t count = worker_list_count();
	while (worker_list_is_running() && count > 0) {
		mpt_trace("There are still %d children running", count);
		if (worker_manager_watchdog(handler, status)) {
			usleep(wait_time);
			count = worker_list_count();
		} else {
			break;
		}
	}
}

bool worker_manager_stop() {
	gru_node_t *node = NULL;
	logger_t logger = gru_logger_get();

	node = worker_list_root();

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
			// otherwise it returns the pid of the process
			if (pid > 0) {
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
					usleep(MPT_MANAGER_SHUTDOWN_WAIT);
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

	worker_list_clean();
	return true;
}