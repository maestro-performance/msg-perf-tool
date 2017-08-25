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
#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <stdio.h>

#include <zlib.h>

#include <common/gru_status.h>
#include <io/gru_ioutils.h>

#include "stats_types.h"
#include "stats_writer.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize the latency writer
 * @param io_info optional input/output information
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_lat_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status);

/**
 * Write latency data
 * @param latency latency data to write
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_lat_writer_write(const stat_latency_t *latency, gru_status_t *status);

/**
 * Flushes latency data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_lat_writer_flush(gru_status_t *status);

/**
 * Finalizes writing latency data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_lat_writer_finalize(gru_status_t *status);

/**
 * Initialize the throughput writer
 * @param io_info optional input/output information
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tp_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status);

/**
 * Write throughput data
 * @param tp throughput data to write
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tp_writer_write(const stat_throughput_t *tp, gru_status_t *status);

/**
 * Flushes throughput data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tp_writer_flush(gru_status_t *status);

/**
 * Finalizes writing latency data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tp_writer_finalize(gru_status_t *status);


/**
 * Initialize the throughput rate writer
 * @param io_info optional input/output information
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tpr_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status);

/**
 * Write throughput rate data with ETA resolution
 * @param tp throughput data to write
 * @param eta estimated time of action (arrival or departure)
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tpr_writer_write(const stat_throughput_t *tp, gru_timestamp_t *eta, gru_status_t *status);

/**
 * Flushes throughput rate data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tpr_writer_flush(gru_status_t *status);

/**
 * Finalizes writing throughput rate data
 * @param status status response in case of error
 * @return true if success or false otherwise (in this case, check status for details)
 */
bool csv_tpr_writer_finalize(gru_status_t *status);

/**
 * Latency writer I/O assignment
 */
void csv_writer_latency_assign(latency_writer_t *writer);

/**
 * Throughput writer I/O assignment
 */
void csv_writer_throughput_assign(throughput_writer_t *writer);


/**
 * Throughput rate writer I/O assignment
 */
void csv_writer_tpr_assign(tpr_writer_t *writer);

#ifdef __cplusplus
}
#endif

#endif /* CSV_WRITER_H */
