/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#ifndef MPT_WORKER_MANAGER_H
#define MPT_WORKER_MANAGER_H

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <sys/wait.h>

#include <common/gru_status.h>

#include "contrib/options.h"
#include "msg_content_data.h"
#include "msgctxt.h"
#include "process_utils.h"
#include "vmsl.h"

#include "maestro/maestro_player.h"
#include "maestro/maestro_sheet.h"
#include "statistics/calculator.h"
#include "statistics/naming_utils.h"
#include "statistics/stats_types.h"
#include "statistics/stats_writer.h"

#include "worker_options.h"
#include "worker_types.h"
#include "worker_utils.h"
#include "worker_info.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPT_MANAGER_SHUTDOWN_WAIT 10000

typedef bool(worker_watchdog_handler)(worker_info_t *worker_info);

typedef worker_ret_t (*worker_start_fn)(const worker_t *worker,
											  worker_snapshot_t *snapshot,
											  gru_status_t *status);

/**
 * Clone a worker
 * @param worker The worker to clone
 * @param worker_start the abstract worker start function (ie:
 * abstract_sender_worker_start)
 * @param status Status container in case of error
 * @return a list of child/clones
 */
gru_list_t *worker_manager_clone(worker_t *worker,
								 worker_start_fn worker_start,
								 gru_status_t *status);

/**
 * Watchdog function that iterates over a list of workers in order to check their status
 * @param list list of workers (as returned by abstract_worker_clone)
 * @param handler A watchdog handler function. A handler function must always return true,
 * otherwise it causes the watchdog to stop running.
 * @return Returns true unless a handler returns false
 */
bool worker_manager_watchdog(gru_list_t *list, worker_watchdog_handler handler);

/**
 * Runs the watchdog loop
 * @param children list of workers (as returned by abstract_worker_clone)
 * @param handler A watchdog handler function. A handler function must always return true,
 * otherwise it causes the watchdog to stop running.
 */
void worker_manager_watchdog_loop(gru_list_t *children, worker_watchdog_handler handler);

/**
 * Stops all workers in the workers list
 * @param list A list of workers to stop
 * @return true unless the list of workers is NULL
 */
bool worker_manager_stop(gru_list_t *list);

#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_MANAGER_H
