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

bool worker_list_start(gru_status_t *status) {
	gwlist = worker_list_new(status);

	return gwlist ? true : false;
}

void worker_list_stop() {
	worker_list_destroy(&gwlist);
}

bool worker_list_append(worker_info_t *winfo, gru_status_t *status) {
	if (!gru_list_append(gwlist->list, winfo)) {
		gru_status_set(status, GRU_FAILURE, "Unable register new worker: %s",
					   status->message);

		return false;
	}

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


gru_node_t *worker_list_remove(gru_node_t *node) {
	gru_node_t *ret = node->next;

	if (!gru_list_remove_node(gwlist->list, node)) {
		logger_t logger = gru_logger_get();

		logger(ERROR, "Unable to remove an orphaned child");
	}

	gru_node_destroy(&node);

	return ret;
}

uint32_t worker_list_count() {
	return gru_list_count(gwlist->list);
}

bool worker_list_is_running() {
	return gwlist && gwlist->list ? true : false;
}