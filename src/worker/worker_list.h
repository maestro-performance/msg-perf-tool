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
#ifndef MPT_WORKER_LIST_H
#define MPT_WORKER_LIST_H

#include <stdlib.h>

#include <common/gru_alloc.h>
#include <common/gru_status.h>
#include <collection/gru_list.h>

#include <common/gru_status.h>

#include "worker_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct worker_list_t worker_list_t;

/**
 * Creates a new worker_list_t
 * @param status status object
 * @return a new worker list or NULL if unable to create one
 */
worker_list_t *worker_list_new(gru_status_t *status);

/**
 * Destroys a worker list
 * @param ptr
 */
void worker_list_destroy(worker_list_t **ptr);

/**
 * Adds a worker to the end of the list
 * @param wlist the list to add the worker to
 * @param winfo the worker information structure
 * @param status status object
 * @return true if successful or false otherwise (in this case, check the status object)
 */
bool worker_list_append(worker_list_t *wlist, worker_info_t *winfo, gru_status_t *status);

/**
 * Gets the root object for the worker list
 * @param wlist the list to get the root object
 * @return
 */
gru_node_t *worker_list_root(worker_list_t *wlist);

/**
 * Remove a worker from the list (stopping its work is the caller responsibility)
 * @param wlist the list to remove from
 * @param node the node containing the worker to remove
 * @return a pointer to the next worker
 */
gru_node_t *worker_list_remove(worker_list_t *wlist, gru_node_t *node);

/**
 * Gets the number of current workers
 * @param wlist the list of workers
 * @return
 */
uint32_t worker_list_count(const worker_list_t *wlist);

/**
 * Returns whether the current worker list is valid/active or not
 * @param wlist
 * @return
 */
bool worker_list_active(const worker_list_t *wlist);

#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_LIST_H
