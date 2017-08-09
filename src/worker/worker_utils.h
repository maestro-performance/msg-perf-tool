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
#ifndef WORKER_UTILS_H
#define WORKER_UTILS_H

#include <common/gru_status.h>
#include <common/gru_alloc.h>

#include "process_utils.h"

#include "worker_options.h"
#include "worker_types.h"
#include "ipc/worker_queue.h"

/**
 * Amount of time to idle when no data is available. After some measurements, it
 * turns out that 25ms seem to provide the right balance between responsiveness
 * and throughput.
 */
#define WORKER_NO_DATA_WAIT 25000

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sets up the messaging options structure (ie.: generates the connection ID and so on)
 * @param opt
 * @param direction
 * @param options
 */
void worker_msg_opt_setup(msg_opt_t *opt, msg_direction_t direction, const worker_options_t *options);

/**
 * Cleans up the messaging options structure
 * @param opt messaging options structure
 */
void worker_msg_opt_cleanup(msg_opt_t *opt);

/**
 * Checks whether to continue processing or not
 */
bool worker_check(const worker_options_t *options, const worker_snapshot_t *snapshot);

/**
 * Formats a worker name based on its details (such as the pid)
 */
const char *worker_name(const worker_t *worker, pid_t child, gru_status_t *status);

/**
 * Creates an IPC queue for child-to-parent data exchange
 */
worker_queue_t *worker_create_queue(const worker_t *worker, gru_status_t *status);


/**
 * Initializes the worker log directory (ie.: /path/to/log/0, /path/to/log/1)
 * @param worker_log_dir The worker log directory
 * @param status a status structure containing error details in case of errors
 * @return true if successful or false otherwise
 */
bool worker_log_init(char *worker_log_dir, gru_status_t *status);


/**
 * Creates the post-execution link directories (ie.: /path/to/lastFailed, /path/to/lastSuccess, etc)
 * @param target target directory containing the logs to be linked (ie.: /path/to/0)
 * @param basedir base log directory (ie.: /path/to/)
 * @param name link name (ie.: lastFailed, last, etc)
 */
void worker_log_link_create(const char *target, const char *basedir, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* WORKER_UTILS_H */
