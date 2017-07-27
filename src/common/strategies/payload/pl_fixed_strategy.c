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
#include "pl_fixed_strategy.h"

static size_t initial_size = 0;

bool pl_fixed_init(msg_content_data_t *data, size_t size, gru_status_t *status) {
	if (size <= 0) {
		gru_status_set(status, GRU_FAILURE, "The message size must be greater than 0");

		return false;
	}

	initial_size = size;
	msg_content_data_init(data, size, status);

	if (!gru_status_success(status)) {
		msg_content_data_release(data);

		return false;
	}

	msg_content_data_fill(data, 'd');
	return true;
}

uint64_t pl_fixed_load(msg_content_data_t *data) {
	return initial_size;
}

void pl_fixed_cleanup(msg_content_data_t *data) {
	msg_content_data_release(data);
}

void pl_fixed_assign(pl_strategy_t *pl_st) {
	pl_st->init = pl_fixed_init;
	pl_st->load = pl_fixed_load;
	pl_st->cleanup = pl_fixed_cleanup;
}