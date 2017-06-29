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
#ifndef CALCULATOR_H_
#define CALCULATOR_H_

#include "stats_types.h"

#include <time/gru_time_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A basic latency calculator
 * @param out output latency data
 * @param start time
 * @param end time
 */
void calc_latency(stat_latency_t *out, gru_timestamp_t start, gru_timestamp_t end);

/**
 * A basic throughput calculator
 * @param out output throughput data
 * @param start time
 * @param end time
 * @param count an arbitrary amount of something (data/messages/events/etc) that ocurred
 * in the interval between start and end
 */
void calc_throughput(stat_throughput_t *out,
	gru_timestamp_t start,
	gru_timestamp_t end,
	uint64_t count);

#ifdef __cplusplus
}
#endif

#endif /* CALCULATOR_H_ */
