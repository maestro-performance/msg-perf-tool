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
#ifndef PL_STRATEGY_H
#define PL_STRATEGY_H

#include <common/gru_status.h>

#include "msg_content_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Payload initialization
 */
typedef bool (*pl_st_init)(msg_content_data_t *data, size_t size, gru_status_t *status);

/**
 * Payload data loading
 */
typedef uint64_t (*pl_st_load)(msg_content_data_t *data);

/**
 * Payload data cleanup
 */
typedef void (*pl_st_cleanup)(msg_content_data_t *data);

/**
 * Virtual payload strategy
 */
typedef struct pl_strategy_t_ {
	pl_st_init init;
	pl_st_load load;
	pl_st_cleanup cleanup;
} pl_strategy_t;

#ifdef __cplusplus
}
#endif

#endif /* PL_STRATEGY_H */
