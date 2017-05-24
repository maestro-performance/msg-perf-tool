/**
 *    Copyright 2017 Otavio Rodolfo Piske
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
 #include "worker_info.h"

worker_info_t *worker_info_new(const worker_t *worker, pid_t child, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	worker_info_t *worker_info = gru_alloc(sizeof(worker_info_t), status);
	gru_alloc_check(worker_info, NULL);

	worker_info->child = child;

	const char *cname = worker_name(worker, child, status);
	if (!cname) {
		kill(child, SIGKILL);
		return NULL;
	}

	logger(INFO, "Created child %s (pid %d) and waiting for the continue signal", cname,
		child);

	worker_wait();

	logger(DEBUG, "Child %d gave the ok signal", child);
	fflush(NULL);

	worker_info->shr =
		shr_buff_new(BUFF_WRITE, sizeof(worker_snapshot_t), cname, status);
	gru_dealloc_const_string(&cname);

	if (!worker_info->shr) {
		gru_status_set(status, GRU_FAILURE, "Unable to open a read buffer: %s",
			status->message);

		kill(child, SIGKILL);
		return NULL;
	}

	return worker_info;
}