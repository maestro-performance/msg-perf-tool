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
#ifndef NAMING_UTILS_H
#define NAMING_UTILS_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <common/gru_status.h>

#include "csv_writer.h"
#include "nop_writer.h"
#include "out_writer.h"
#include "hdr_writer.h"

#include "stats_writer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum report_format_t_ {
	FORMAT_CSV,
	FORMAT_NOP,
	FORMAT_OUT,
	FORMAT_HDR,
} report_format_t;

typedef enum naming_opt_t_ {
	NM_NONE = 0,
	NM_LATENCY = 1,
	NM_THROUGHPUT = 2,
  	NM_RATE = 4,
} naming_opt_t;

typedef struct naming_info_t_ {
	char *source; /** "sender" || "receiver" || "bmic" */
	const char *location; /** Log path */
} naming_info_t;

bool naming_initialize_writer(stats_writer_t *writer,
	report_format_t format,
	naming_opt_t nm_opt,
	void *payload,
	gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* NAMING_UTILS_H */
