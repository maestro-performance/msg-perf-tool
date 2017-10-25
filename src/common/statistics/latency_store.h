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

#ifndef MPT_LATENCY_STORE_H
#define MPT_LATENCY_STORE_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include <common/gru_status.h>
#include <log/gru_logger.h>

bool latency_store_init_count(uint64_t count, int32_t parallel_count, gru_status_t *status);
bool latency_store_init_duration(uint64_t seconds, int32_t parallel_count, gru_status_t *status);
void latency_store_release();
bool latency_store_add(int64_t value);
double latency_store_calc_percentile(double percentile);

#endif //MPT_LATENCY_STORE_H
