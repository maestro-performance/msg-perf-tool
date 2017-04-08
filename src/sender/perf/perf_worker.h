/**
 Copyright 2016 Otavio Rodolfo Piske

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#ifndef PERF_WORKER_H
#define PERF_WORKER_H

#include "contrib/options.h"
#include "msgctxt.h"
#include "msgdata.h"
#include "process_utils.h"
#include "statistics.h"
#include "vmsl.h"

#include <inttypes.h>
#include <unistd.h>

#include <common/gru_status.h>

#ifdef __cplusplus
extern "C" {
#endif

void perf_worker_start(const vmsl_t *vmsl, const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* PERF_WORKER_H */
