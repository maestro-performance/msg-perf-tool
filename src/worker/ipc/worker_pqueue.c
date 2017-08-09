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
#include "worker_pqueue.h"

struct worker_pqueue_t_ {
  char *name;
  mqd_t queue;
};

worker_pqueue_t *worker_pqueue_new(const char *name, queue_perm_t perm, long msg_size,  gru_status_t *status) {
	worker_pqueue_t *ret = gru_alloc(sizeof(worker_pqueue_t), status);
	gru_alloc_check(ret, NULL);

	if (asprintf(&ret->name, "/%s", name) == -1) {
		gru_status_set(status, GRU_FAILURE, "Unable to format posix queue name");

		return NULL;
	}

	int default_perm = O_RDWR | O_NONBLOCK | O_CLOEXEC | O_CREAT;

	struct mq_attr attr;

	attr.mq_maxmsg = 65535;
	attr.mq_flags = O_NONBLOCK;
	attr.mq_msgsize = msg_size;

	ret->queue = mq_open(ret->name, default_perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
						&attr);
	if (ret->queue == (mqd_t) -1) {
		logger_t logger = gru_logger_get();

		logger(GRU_ERROR, "Error = %s", strerror(errno));
		gru_status_strerror(status, GRU_FAILURE, errno);

		worker_pqueue_destroy(&ret);
	}

	return ret;
}

void worker_pqueue_destroy(worker_pqueue_t **ptr) {
	worker_pqueue_t *worker_pqueue = *ptr;

	if (!worker_pqueue) {
		return;
	}

	gru_dealloc_string(&worker_pqueue->name);
	if (worker_pqueue->queue != (mqd_t) -1) {
		mq_close(worker_pqueue->queue);
	}

	gru_dealloc((void **) ptr);
}

worker_queue_stat_t worker_pqueue_write(const worker_pqueue_t * const worker_pqueue, const void *data, size_t len, void *payload) {
	const int default_prio = 0;

	if (mq_send(worker_pqueue->queue, data, len, default_prio) == 0) {
		return MPT_WQ_SUCCESS;
	}

	logger_t  logger = gru_logger_get();
	logger(GRU_ERROR, "Write error %s", strerror(errno));
	return MPT_WQ_ERROR;
}

worker_queue_stat_t worker_pqueue_read(const worker_pqueue_t * const worker_pqueue, void *dest, size_t len) {
	unsigned int prio;

	ssize_t ret = mq_receive(worker_pqueue->queue, dest, len, &prio);
	if (ret >= 0) {
		return MPT_WQ_SUCCESS;
	}
	else {
		if (errno == EAGAIN) {
			return MPT_WQ_SUCCESS | MPT_WQ_NO_DATA;
		}
	}

	logger_t  logger = gru_logger_get();
	logger(GRU_ERROR, "Read error %s", strerror(errno));

	return MPT_WQ_ERROR;
}