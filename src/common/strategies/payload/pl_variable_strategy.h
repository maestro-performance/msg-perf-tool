/**
 *    Copyright 2017 Otavio Rodolfo Piske
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this fileVARIABLEpt in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, eitherVARIABLEess or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef PL_VARIABLE_STRATEGY_H
#define PL_VARIABLE_STRATEGY_H

#ifdef __cplusplus
#endif

#include <common/gru_status.h>

#include "msg_content_data.h"
#include "strategies/payload/pl_strategy.h"

/**
 * This "strategy" is used for variable-size payloads (ie.: non-deterministic message sizes)
 */

bool pl_variable_init(msg_content_data_t *data, size_t size, gru_status_t *status);
uint64_t pl_variable_load(msg_content_data_t *data);
void pl_variable_cleanup(msg_content_data_t *data);

void pl_variable_assign(pl_strategy_t *pl_st);

#ifdef __cplusplus
}
#endif


#endif /* PL_VARIABLE_STRATEGY_H */
