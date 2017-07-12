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
#ifndef MPT_WORKER_HANDLER_H
#define MPT_WORKER_HANDLER_H

#include <stdbool.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "worker_types.h"
#include "worker_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A signature for the eval handler. Must return true if the condition is true,
 * in which case the test must continue. Returning false causes the test to stop
 */
typedef bool(*worker_eval)(worker_info_t *worker_info, gru_status_t *status);


/**
 * A signrature for the print handler
 */
typedef bool(*worker_print)(worker_info_t *worker_info);

/**
 * The handle flags affecting the handler structure
 */
typedef enum worker_handler_flags_t_ {
  	WRK_HANDLE_NONE = 0, /** NO OP handler */
	WRK_HANDLE_EVAL = 1, /** Eval handler (ie.: enables test condition checks) */
  	WRK_HANDLE_PRINT = 2 /** Print handler (ie.: print current test statistics) */
} worker_handler_flags_t;

/**
 * The handler structure for the worker data. The worker handler is executed every
 * iteration of the worker watchdog.
 */
typedef struct worker_handler_t_ {
  worker_handler_flags_t flags;
  worker_eval eval;
  worker_print print;
} worker_handler_t ;

#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_HANDLER_H
