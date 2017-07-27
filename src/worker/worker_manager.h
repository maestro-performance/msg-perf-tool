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
#include "worker_list.h"
#include "worker_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPT_MANAGER_SHUTDOWN_WAIT 100000


typedef worker_ret_t (*worker_start_fn)(const worker_t *worker,
											  worker_snapshot_t *snapshot,
											  gru_status_t *status);

/**
 * Clone worker up to the count in the worker options structure
 * @param worker The worker to clone
 * @param worker_start the abstract worker start function (ie:
 * abstract_sender_worker_start)
 * @param status Status container in case of error
 * @return true if successfully cloned the workers or false otherwise
 */
worker_ret_t worker_manager_clone(worker_t *worker,
								 worker_start_fn worker_start,
								 gru_status_t *status);


/**
 * Runs the watchdog loop
 * @param handler A watchdog handler function. A handler function must always return true,
 * otherwise it causes the watchdog to stop running.
 */
void worker_manager_watchdog_loop(worker_handler_t *handler, gru_status_t *status);

/**
 * Stops all workers in the workers list
 * @return true unless the list of workers is NULL
 */
bool worker_manager_stop();


#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_MANAGER_H
