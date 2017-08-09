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
#include "worker_vqueue.h"
#include "../worker_types.h"
#include "process_utils.h"

struct worker_vqueue_t_ {
  char *name;
  int queue;
};

worker_vqueue_t *worker_vqueue_new(const char *name, queue_perm_t perm, gru_status_t *status) {
	worker_vqueue_t *ret = gru_alloc(sizeof(worker_vqueue_t), status);
	gru_alloc_check(ret, NULL);

	if (asprintf(&ret->name, "/%s", name) == -1) {
		gru_status_set(status, GRU_FAILURE, "Unable to format posix queue name");

		return NULL;
	}

	key_t key = ftok("/tmp", 0x32);
	ret->queue = create_queue(key, status);

	if (ret->queue < 0) {
		worker_vqueue_destroy(&ret);
		return NULL;
	}

	return ret;
}

void worker_vqueue_destroy(worker_vqueue_t **ptr) {
	worker_vqueue_t *worker_pqueue = *ptr;

	if (!worker_pqueue) {
		return;
	}

	gru_dealloc_string(&worker_pqueue->name);
	if (worker_pqueue->queue != -1) {
		//
	}

	gru_dealloc((void **) ptr);
}

struct worker_ipc_buff_t {
	long mtype;
	char mtext[256];
};

worker_queue_stat_t worker_vqueue_write(const worker_vqueue_t *const worker_pqueue, const void *data, size_t len, void *payload) {
	struct worker_ipc_buff_t buff = {0};

	mpt_trace("Requesting to write %d bytes", len);

	buff.mtype = 1;
	memcpy(buff.mtext, data, len);

	if (msgsnd(worker_pqueue->queue, &buff, len, IPC_NOWAIT) == 0) {
		return MPT_WQ_SUCCESS;
	}

	logger_t  logger = gru_logger_get();
	logger(GRU_ERROR, "SysV queue write error %s", strerror(errno));
	return MPT_WQ_ERROR;
}

worker_queue_stat_t worker_vqueue_read(const worker_vqueue_t *const worker_pqueue, void *dest, size_t len) {
	if (!worker_pqueue) {
		return MPT_WQ_ERROR;
	}

	struct worker_ipc_buff_t buff = {0};

	if (len > sizeof(buff.mtext)) {
		logger_t logger = gru_logger_get();

		logger(GRU_ERROR, "The size of the output buffer is greater that the IPC queue buffer: %d > %d",
			len, sizeof(buff.mtext));

		return MPT_WQ_ERROR;
	}

	mpt_trace("Requesting to read %d bytes", len);

	ssize_t ret = msgrcv(worker_pqueue->queue, &buff, sizeof(buff.mtext), 0, IPC_NOWAIT);
	if (ret >= 0) {
		memcpy(dest, buff.mtext, len);

		return MPT_WQ_SUCCESS;
	}

	if (errno == ENOMSG) {
		mpt_trace("No message to read");
		return MPT_WQ_SUCCESS | MPT_WQ_NO_DATA;
	}

	logger_t  logger = gru_logger_get();
	logger(GRU_ERROR, "SysV queue read error %s", strerror(errno));

	return MPT_WQ_ERROR;
}