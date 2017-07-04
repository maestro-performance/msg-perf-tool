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
#ifndef STATS_TYPES_H
#define STATS_TYPES_H

#include <stdint.h>

#include <time/gru_duration.h>

/**
 * Stats structure and data version
 */
typedef enum stats_version_t_ {
  MPT_STATS_V1
} stats_version_t;

/**
 * A place holder for the latency information
 */
typedef struct stat_latency_t_ {
  	gru_duration_t duration;
	gru_timestamp_t elapsed;
  	stats_version_t version;
} stat_latency_t;

/**
 * A place holder for the throughput information
 */
typedef struct stat_throughput_t_ {
	gru_duration_t duration;
	uint64_t count;
	double rate;
  	stats_version_t version;
} stat_throughput_t;

#endif /* STATS_TYPES_H */
