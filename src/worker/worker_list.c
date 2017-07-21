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
#include "worker_list.h"

struct worker_list_t {
  gru_list_t *list;
};

static worker_list_t *gwlist;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void worker_list_destroy(worker_list_t **ptr) {
	worker_list_t *tmp_list = *ptr;

	if (!tmp_list) {
		return;
	}

	if (tmp_list->list) {
		gru_list_clean(tmp_list->list, worker_info_destroy_wrapper);
		gru_list_destroy(&tmp_list->list);
	}

	gru_dealloc((void **) ptr);
}

static worker_list_t *worker_list_new(gru_status_t *status) {
	worker_list_t *ret = gru_alloc(sizeof(worker_list_t), status);
	gru_alloc_check(ret, NULL);

	ret->list = gru_list_new(status);
	if (!ret->list) {
		worker_list_destroy(&ret);


		return NULL;
	}


	return ret;
}

// Not thread safe
bool worker_list_init(gru_status_t *status) {
	if (gwlist) {
		return false;
	}

	gwlist = worker_list_new(status);
	return gwlist ? true : false;
}

void worker_list_clean() {
	pthread_mutex_lock(&mutex);

	worker_list_destroy(&gwlist);

	pthread_mutex_unlock(&mutex);
}

bool worker_list_append(worker_info_t *winfo, gru_status_t *status) {
	pthread_mutex_lock(&mutex);

	if (!gwlist || !gwlist->list) {
		pthread_mutex_unlock(&mutex);

		return false;
	}

	if (!gru_list_append(gwlist->list, winfo)) {
		gru_status_set(status, GRU_FAILURE, "Unable register new worker: %s",
					   status->message);

		pthread_mutex_unlock(&mutex);
		return false;
	}

	pthread_mutex_unlock(&mutex);
	return true;
}

gru_node_t *worker_list_root() {
	if (gwlist) {
		if (gwlist->list) {
			return gwlist->list->root;
		}
	}

	return NULL;
}


gru_node_t *worker_list_remove_unlocked(gru_node_t *node) {
	gru_node_t *ret = node->next;

	if (!gru_list_remove_node(gwlist->list, node)) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Unable to remove an orphaned child");
	}

	gru_node_destroy(&node);

	return ret;
}


uint32_t worker_list_count() {
	uint32_t ret = 0;
	pthread_mutex_lock(&mutex);

	if (gwlist && gwlist->list) {
		 ret = gru_list_count(gwlist->list);
	}

	pthread_mutex_unlock(&mutex);

	return ret;
}

bool worker_list_is_running() {
	return gwlist && gwlist->list ? true : false;
}

void worker_list_lock() {
	pthread_mutex_lock(&mutex);
}

void worker_list_unlock() {
	pthread_mutex_unlock(&mutex);
}