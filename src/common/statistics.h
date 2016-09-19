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
#ifndef STATISTICS_H
#define STATISTICS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "timestamp.h"

#include <common/gru_status.h>
#include <io/gru_ioutils.h>
#include <log/gru_logger.h>


#include <unistd.h>
#include <time.h>
#include <inttypes.h>

typedef enum stat_direction_t_ {
    SENDER,
    RECEIVER,
} stat_direction_t;

typedef struct stat_io_t_ {
    FILE *latency;
    FILE *throughput;
    stat_direction_t direction;
} stat_io_t;

stat_io_t *statistics_init(stat_direction_t direction, gru_status_t *status);
void statistics_destroy(stat_io_t **stat_io);

void statistics_latency_header(stat_io_t *stat_io);
void statistics_throughput_header(stat_io_t *stat_io);

void statistics_latency_data(stat_io_t *stat_io, uint64_t creation, 
                             const char *time, int32_t milli);
void statistics_throughput_data(stat_io_t *stat_io, const char *last_buff, 
                                uint64_t count, uint64_t partial, double rate);

void statistics_latency(stat_io_t *stat_io, mpt_timestamp_t start, mpt_timestamp_t end);
uint64_t statistics_diff(mpt_timestamp_t start, mpt_timestamp_t end);
void statistics_throughput_partial(stat_io_t *stat_io, mpt_timestamp_t start, 
                                   mpt_timestamp_t last, 
                                   uint64_t count);


#ifdef __cplusplus
}
#endif

#endif /* STATISTICS_H */

