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
#include "worker_queue.h"

enum worker_queue_type {
  MPT_Q_POSIX,
  MPT_Q_SYSV
};

struct worker_queue_t_ {
  enum worker_queue_type type;
  union {
	worker_vqueue_t *vqueue;
	worker_pqueue_t *pqueue;
  } via;
};


worker_queue_t *worker_queue_new(const char *name, queue_perm_t perm, long msg_size, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	worker_queue_t *ret = gru_alloc(sizeof(worker_queue_t), status);
	gru_alloc_check(ret, NULL);

	char *use_pqueue = getenv("MPT_USE_POSIX_QUEUE");

	if (use_pqueue) {
		logger(GRU_INFO, "Using POSIX queues for IPC");
		ret->via.pqueue = worker_pqueue_new(name, perm, msg_size, status);
		ret->type = MPT_Q_POSIX;
	}
	else {
		logger(GRU_INFO, "Using SysV queues for IPC");
		ret->via.vqueue = worker_vqueue_new(name, perm, status);
		ret->type = MPT_Q_SYSV;
	}

	return ret;
}
void worker_queue_destroy(worker_queue_t **ptr) {
	worker_queue_t *worker_queue = *ptr;

	if (!worker_queue) {
		return;
	}

	if (worker_queue->type == MPT_Q_SYSV) {
		worker_vqueue_destroy(&worker_queue->via.vqueue);
	}
	else {
		worker_pqueue_destroy(&worker_queue->via.pqueue);
	}

	gru_dealloc((void **) ptr);
}

worker_queue_stat_t worker_queue_write(const worker_queue_t *const worker_queue, const void *data, size_t len, void *payload) {
	if (worker_queue->type == MPT_Q_SYSV) {
		return worker_vqueue_write(worker_queue->via.vqueue, data, len, payload);
	}
	else {
		return worker_pqueue_write(worker_queue->via.pqueue, data, len, payload);
	}

}
worker_queue_stat_t worker_queue_read(const worker_queue_t *const worker_queue, void *dest, size_t len) {
	if (worker_queue->type == MPT_Q_SYSV) {
		return worker_vqueue_read(worker_queue->via.vqueue, dest, len);
	}
	else {
		return worker_pqueue_read(worker_queue->via.pqueue, dest, len);
	}
}