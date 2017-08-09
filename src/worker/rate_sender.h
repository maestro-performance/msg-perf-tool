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
#ifndef MPT_RATE_SENDER_H
#define MPT_RATE_SENDER_H

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
#include "ipc/worker_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Execute a rate-based sender worker
 * @param worker The worker to execute
 * @param snapshot The last measured snapshot for the worker. Callers of this function can
 * use
 * the address of this variable to check the current status of the test execution
 * @param status Status container in case of error
 * @param WORKER_SUCCESS if successful or a composed value (including WORKER_FAILURE) in
 * case of errors
 */
worker_ret_t rate_sender_start(const worker_t *worker,
								worker_snapshot_t *snapshot,
								gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif //MPT_RATE_SENDER_H
