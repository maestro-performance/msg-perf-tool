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
#ifndef WORKER_WAIT_H
#define WORKER_WAIT_H

#include <signal.h>
#include <sys/wait.h>

#include "process_utils.h"

#include "worker_options.h"
#include "worker_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Setups the worker wait
 */
void worker_wait_setup();

/**
 * Wait until the worker is ready
 */
void worker_wait();

#ifdef __cplusplus
}
#endif

#endif /* WORKER_WAIT_H */
