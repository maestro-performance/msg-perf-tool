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

#ifndef BMIC_WRITER_H
#define BMIC_WRITER_H

#include <stdio.h>

#include <zlib.h>

#include <common/gru_status.h>
#include <common/gru_units.h>
#include <io/gru_ioutils.h>
#include <time/gru_time_utils.h>

#include <context/bmic_context.h>

#include "bmic_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Writer utilities for the data returned from BMIC
 */

/**
 * Initialize the writer
 */
bool bmic_writer_initialize(const char *dir, const char *name, gru_status_t *status);

/**
 * Flushes current data to disk
 */
bool bmic_writer_flush(gru_status_t *status);

/**
 * Writes current time
 */
bool bmic_writer_current_time(gru_status_t *status);

/**
 * Writes OS info
 */
void bmic_writer_osinfo(const bmic_java_os_info_t *osinfo);

/** 
 * Writes java memory
 */
void bmic_writer_java_mem(const mpt_java_mem_t *jmeminfo, bmic_java_memory_model_t memory_model);

/**
 * Writes queue statistics
 */
void bmic_writer_queue_stat(const bmic_queue_stat_t *stat);

/**
 * Finalize the writer
 */
bool bmic_writer_finalize(gru_status_t *status);

#ifdef __cplusplus
}
#endif


#endif /* BMIC_WRITER_H */
