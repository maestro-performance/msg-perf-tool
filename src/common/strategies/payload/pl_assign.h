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

#ifndef PL_ASSIGN_H
#define PL_ASSIGN_H

#include "strategies/payload/pl_fixed_strategy.h"
#include "strategies/payload/pl_strategy.h"
#include "strategies/payload/pl_variable_strategy.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Assign the appropriate payload strategy
 */
static inline void pl_strategy_assign(pl_strategy_t *pl_strategy, bool variable_size) {
	if (variable_size) {
		pl_variable_assign(pl_strategy);
	} else {
		pl_fixed_assign(pl_strategy);
	}
}

#ifdef __cplusplus
}
#endif

#endif /* PL_ASSIGN_H */
