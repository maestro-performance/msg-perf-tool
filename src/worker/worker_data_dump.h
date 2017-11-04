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
#ifndef MPT_WORKER_DATA_DUMP_H
#define MPT_WORKER_DATA_DUMP_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <config/gru_config.h>

#include "vmsl.h"
#include "worker_options.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dump the worker options so it can be processed later
 * @param dir
 * @param options
 * @param info
 * @param status
 * @return
 */
bool worker_dump(const char *dir, worker_options_t *options, vmsl_info_t *info, gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_DATA_DUMP_H
