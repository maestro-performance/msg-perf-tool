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
#ifndef WORKER_INFO_H
#define WORKER_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <common/gru_status.h>
#include <common/gru_alloc.h>

#include "worker_types.h"
#include "worker_utils.h"
#include "worker_options.h"

/**
 * Creates a new worker information structure
 * @param worker the worker to store information about
 * @param child the PID of the worker
 * @param status the status holder
 * @return A pointer to a worker information structure of NULL if not found
 */
worker_info_t *worker_info_new(const worker_t *worker, pid_t child, gru_status_t *status);

void worker_info_destroy(worker_info_t **ptr);

static inline void worker_info_destroy_wrapper(void **ptr) {
	worker_info_destroy((worker_info_t **) ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* WORKER_INFO_H */
