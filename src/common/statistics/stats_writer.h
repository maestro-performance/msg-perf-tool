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
#ifndef STATS_WRITER_H
#define STATS_WRITER_H

#include "stats_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stats source information
 */
typedef struct stat_source_info_t_ { char *name; /** Source name */ } stat_source_info_t;

/**
 * Stats data destination information
 */
typedef struct stat_dest_info_t_ {
	char *location; /** Location: path, URL, etc */
	char *name; /** Destination name */
} stat_dest_info_t;

/**
 * Combined I/O information structure
 */
typedef struct stat_io_info_t_ {
	stat_source_info_t source;
	stat_dest_info_t dest;
  	stats_version_t version;
} stat_io_info_t;

/**
 * Initialize the latency writer
 * @param io_info optional input/output information
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (
	*lat_writer_initialize)(const stat_io_info_t *io_info, gru_status_t *status);

/**
 * Write latency data
 * @param latency latency data to write
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*lat_writer_write)(const stat_latency_t *latency, gru_status_t *status);

/**
 * Flushes latency data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*lat_writer_flush)(gru_status_t *status);

/**
 * Finalizes writing latency data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*lat_writer_finalize)(gru_status_t *status);

/**
 * Initialize the throughput writer
 * @param io_info optional input/output information
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tp_writer_initialize)(const stat_io_info_t *io_info, gru_status_t *status);


/**
 * Write throughput data
 * @param tp throughput data to write
  * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tp_writer_write)(const stat_throughput_t *tp, gru_status_t *status);


/**
 * Flushes throughput data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tp_writer_flush)(gru_status_t *status);

/**
 * Finalizes writing throughput data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tp_writer_finalize)(gru_status_t *status);


/**
 * Initialize the throughput rate writer
 * @param io_info optional input/output information
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tpr_writer_initialize)(const stat_io_info_t *io_info, gru_status_t *status);

/**
 * Write throughput rate data with ETD resolution
 * @param tp throughput data to write
 * @param eta estimated time of action (ETD for departure or ETA for arrival)
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tpr_writer_write)(const stat_throughput_t *tp, gru_timestamp_t *eta, gru_status_t *status);

/**
 * Flushes throughput rate
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tpr_writer_flush)(gru_status_t *status);

/**
 * Finalizes writing throughput rate
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
typedef bool (*tpr_writer_finalize)(gru_status_t *status);


/**
 * Latency writer
 */
typedef struct latency_writer_t_ {
	lat_writer_initialize initialize;
	lat_writer_write write;
	lat_writer_flush flush;
	lat_writer_finalize finalize;
} latency_writer_t;

/**
 * Throughput writer
 */
typedef struct throughput_writer_t_ {
	tp_writer_initialize initialize;
	tp_writer_write write;
  	tp_writer_flush flush;
	tp_writer_finalize finalize;
} throughput_writer_t;


/**
 * Throughput rate writer
 */
typedef struct tpr_writer_t_ {
  tpr_writer_initialize initialize;
  tpr_writer_write write;
  tpr_writer_flush flush;
  tpr_writer_finalize finalize;
} tpr_writer_t;

typedef struct stats_writer_t_ {
	latency_writer_t latency;
	throughput_writer_t throughput;
  	tpr_writer_t rate;
} stats_writer_t;

#ifdef __cplusplus
}
#endif

#endif /* STATS_WRITER_H */
