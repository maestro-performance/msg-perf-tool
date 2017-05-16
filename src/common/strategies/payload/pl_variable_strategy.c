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
#include "pl_variable_strategy.h"

static size_t initial_size = 0;
static size_t lower_bound = 0;
static size_t upper_bound = 0;
static size_t limit = 0;

bool pl_variable_init(msg_content_data_t *data, size_t size, gru_status_t *status) {
	initial_size = size;

	if (size > 100) {
		size_t bound = ((size / 100) * 5);

		lower_bound = size - bound;
		upper_bound = size + bound;
		limit = upper_bound - lower_bound;
	} else {
		lower_bound = size - 1;
		upper_bound = size + 1;
		limit = upper_bound - lower_bound;
	}

	msg_content_data_init(data, upper_bound, status);

	if (!gru_status_success(status)) {
		msg_content_data_release(data);

		return false;
	}

	msg_content_data_rfill(data);
	return true;
}

uint64_t pl_variable_load(msg_content_data_t *data) {
	data->size = lower_bound + (rand() % limit);

	return initial_size;
}

void pl_variable_cleanup(msg_content_data_t *data) {
	msg_content_data_release(data);
}

void pl_variable_assign(pl_strategy_t *pl_st) {
	pl_st->init = pl_variable_init;
	pl_st->load = pl_variable_load;
	pl_st->cleanup = pl_variable_cleanup;
}