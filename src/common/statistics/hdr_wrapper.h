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
#ifndef MPT_HDR_WRAPPER_H
#define MPT_HDR_WRAPPER_H

#include <stdio.h>

#include <hdr/hdr_histogram.h>
#include <hdr/hdr_histogram_log.h>
#include <hdr/hdr_interval_recorder.h>
#include <hdr/hdr_time.h>

#include <common/gru_status.h>
#include <io/gru_ioutils.h>

#include "stats_types.h"
#include "stats_writer.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * A wrapper struct around HDR Histogram C types
 */
typedef struct hdr_wrapper_t_ {
  struct hdr_interval_recorder recorder;
  struct hdr_log_writer writer;
  struct timespec start_timestamp;
  FILE *file;
} hdr_wrapper_t;


/**
 * Initializes the wrapper and the HDR Histogram recorder
 * @param io_info
 * @param wrapper
 * @param status
 * @return
 */
bool hdr_wrapper_initialize(const stat_io_info_t *io_info, hdr_wrapper_t *wrapper, gru_status_t *status);

/**
 * Finalize the wrapper and the HDR Histogram recorder
 * @param wrapper
 */
void hdr_wrapper_finalize(hdr_wrapper_t *wrapper);

#ifdef __cplusplus
}
#endif

#endif //MPT_HDR_WRAPPER_H
