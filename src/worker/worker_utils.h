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

#ifdef __cplusplus
extern "C" {
#endif

#include "process_utils.h"

#include "worker_options.h"
#include "worker_types.h"

bool worker_check(const worker_options_t *options, const worker_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* WORKER_UTILS_H */
