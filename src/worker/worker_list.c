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

worker_list_t *worker_list_new(gru_status_t *status) {
	worker_list_t *ret = gru_alloc(sizeof(worker_list_t), status);
	gru_alloc_check(ret, NULL);

	ret->list = gru_list_new(status);
	if (!ret->list) {
		worker_list_destroy(&ret);

		return NULL;
	}

	return ret;
}

void worker_list_destroy(worker_list_t **ptr) {
	worker_list_t *wlist = *ptr;

	if (!wlist) {
		return;
	}

	if (wlist->list) {
		gru_list_clean(wlist->list, worker_info_destroy_wrapper);
		gru_list_destroy(&wlist->list);
	}

	gru_dealloc((void **) ptr);
}

bool worker_list_append(worker_list_t *wlist, worker_info_t *winfo, gru_status_t *status) {
	if (!gru_list_append(wlist->list, winfo)) {
		gru_status_set(status, GRU_FAILURE, "Unable register new worker: %s",
					   status->message);

		return false;
	}

	return true;
}

gru_node_t *worker_list_root(worker_list_t *wlist) {
	if (wlist) {
		if (wlist->list) {
			return wlist->list->root;
		}
	}

	return NULL;
}

gru_node_t *worker_list_remove(worker_list_t *wlist, gru_node_t *node) {
	gru_node_t *ret = node->next;

	if (!gru_list_remove_node(wlist->list, node)) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Unable to remove an orphaned child");
	}

	gru_node_destroy(&node);

	return ret;
}

uint32_t worker_list_count(const worker_list_t *wlist) {
	return gru_list_count(wlist->list);
}

bool worker_list_active(const worker_list_t *wlist) {
	return wlist && wlist->list ? true : false;
}