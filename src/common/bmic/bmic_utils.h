/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#ifndef BMIC_UTILS_H
#define BMIC_UTILS_H

#include <stdlib.h>
#include <stdbool.h>

#include <common/gru_status.h>
#include <network/gru_uri.h>

#include <context/bmic_context.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpt_java_mem_t_ {
	bmic_java_mem_info_t eden;
	bmic_java_mem_info_t survivor;
	bmic_java_mem_info_t tenured;
	bmic_java_mem_info_t metaperm;
} mpt_java_mem_t;

/**
 * Initializes the bmic context
 */
bool mpt_init_bmic_ctxt(gru_uri_t uri, bmic_context_t *ctxt, 
	gru_status_t *status);

/**
 * Read queue stats
 */
void mpt_get_queue_stats(const bmic_context_t *ctxt, const char *name, 
	bmic_queue_stat_t *stat, gru_status_t *status);

/**
  * Read memory information
  */
void mpt_get_mem_info(const bmic_context_t *ctxt, bmic_java_memory_model_t memory_model, 
	mpt_java_mem_t *out, gru_status_t *status);

/**
 * Purge the queue and reset its counters
 */
bool mpt_purge_queue(const bmic_context_t *ctxt, const char *name, gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* BMIC_UTILS_H */
