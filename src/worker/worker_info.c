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

	worker_info_t *ret = gru_alloc(sizeof(worker_info_t), status);
	gru_alloc_check(ret, NULL);

	ret->child = child;

	const char *cname = worker_name(worker, child, status);
	if (!cname) {
		kill(child, SIGKILL);
		return NULL;
	}

	logger(GRU_DEBUG, "Child %d gave the ok signal", child);
	fflush(NULL);


	worker_queue_opt_t queue_opt = worker_queue_new_opt(worker->worker_flags);

	ret->pqueue = worker_queue_new(cname, PQUEUE_READ, queue_opt, status);

	gru_dealloc_const_string(&cname);

	if (!ret->pqueue) {
		gru_status_set(status, GRU_FAILURE, "Unable to create a posix queue: %s",
					   status->message);

		kill(child, SIGKILL);
		return NULL;
	}

	ret->worker_flags = worker->worker_flags;
	ret->writer = worker->writer;

	return ret;
}



void worker_info_destroy(worker_info_t **ptr) {
	worker_info_t *wi = *ptr;

	if (!wi) {
		return;
	}

	worker_queue_destroy(&wi->pqueue);
	gru_dealloc((void **) ptr);
}