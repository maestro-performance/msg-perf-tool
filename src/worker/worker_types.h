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

#ifndef WORKER_TYPES_H
#define WORKER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time/gru_duration.h>

#include "vmsl.h"
#include "statistics/stats_types.h"
#include "statistics/stats_writer.h"
#include "statistics/naming_utils.h"
#include "strategies/payload/pl_strategy.h"

#ifdef MPT_SHARED_BUFFERS
 #include "ipc/shared_data_buffer.h"
#endif // MPT_SHARED_BUFFERS

/**
 * Common return type for the worker
 */
typedef enum worker_ret_t_ {
	WORKER_FAILURE,
	WORKER_SUCCESS,
} worker_ret_t;

/**
 * Worker flags 
 */
typedef enum worker_flags_t_ {
	WRK_NONE = 0, /** None */
	WRK_RECEIVER = 1, /** Data receiver worker */
	WRK_SENDER = 2, /** Data sender worker */
	WRK_DAEMON = 4, /** Worker runs as a daemon */
	WRK_FORKED = 8, /** Worked forked from a controller process */
} workder_flags_t;

/**
 * Represents a snapshot of the current work iteration
 */
typedef struct worker_snapshot_t_ {
	uint64_t count;
	gru_timestamp_t start;
	gru_timestamp_t now;
	stat_latency_t latency;
	stat_throughput_t throughput;
} worker_snapshot_t;

typedef bool(*worker_iteration_check)(const worker_options_t *options, 
	const worker_snapshot_t *snapshot);

/**
 * Abstracts the "operational" parts of the test execution, options, etc.
 */ 
typedef struct worker_t_ {
	char *name;
	const vmsl_t *vmsl;
	worker_options_t *options;
	stats_writer_t *writer;
	report_format_t report_format;
	naming_opt_t naming_options;
	worker_iteration_check can_continue;
	workder_flags_t worker_flags;
	pl_strategy_t pl_strategy;
} worker_t;

typedef struct worker_info_t_ {
	pid_t child;
	worker_snapshot_t snapshot;
	
#ifdef MPT_SHARED_BUFFERS
 	volatile shr_data_buff_t *shr;
#endif // MPT_SHARED_BUFFERS

} worker_info_t;

#ifdef __cplusplus
}
#endif


#endif /* WORKER_TYPES_H */
