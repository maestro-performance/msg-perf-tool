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
#include "latency_store.h"

static int64_t *latencies = NULL;
static uint64_t capacity = 0;
static uint64_t current = 0;

static int32_t increment_size = 10;

static bool latency_store_do_init(gru_status_t *status) {
	latencies = calloc(capacity, sizeof(int64_t));
	if (!latencies) {
		gru_status_set(status, GRU_FAILURE, "Unable to create the latencies store: not enough memory");

		return false;
	}

	return true;
}


bool latency_store_init_count(uint64_t count, int32_t parallel_count, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	// Add some extra buffer in case some messages are delivered later
	capacity = (count * parallel_count);
	capacity += (capacity / increment_size);

	logger(GRU_DEBUG, "Creating a count-based store for %"PRIu64" messages", capacity);

	return latency_store_do_init(status);
}

bool latency_store_init_duration(uint64_t seconds, int32_t parallel_count, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	uint64_t max_tp = 40000L;
	capacity = seconds * max_tp * parallel_count;
	capacity += (capacity / increment_size);

	logger(GRU_DEBUG, "Creating a time-based store for %"PRIu64" messages", capacity);

	return latency_store_do_init(status);
}


void latency_store_release() {
	free(latencies);
	latencies = NULL;
}

static int latency_store_comp(const void *p1, const void *p2) {
	int64_t *lat1 = (int64_t *) p1;
	int64_t *lat2 = (int64_t *) p2;

	if (*lat1 < *lat2) {
		return -1;
	}
	else {
		if (*lat1 > *lat2) {
			return 1;
		}
	}

	return 0;
}



bool latency_store_add(int64_t value) {
	if (current == (capacity - 1)) {
		logger_t logger = gru_logger_get();

		uint64_t new_capacity = capacity + (capacity / increment_size);
		int64_t *new_ptr = realloc(latencies, (new_capacity * sizeof(int64_t)));
		if (new_ptr == NULL) {
			logger(GRU_ERROR, "Not enough memory to increase store to %"PRIu64"", new_capacity);
			return false;
		}
		capacity = new_capacity;
		latencies = new_ptr;

		latencies[current] = value;
		logger(GRU_DEBUG, "Increased current store capacity to %"PRIu64"", capacity);
	}

	latencies[current] = value;
	current++;
	return true;
}

static void latency_store_sort() {
	qsort(latencies, current, sizeof(int64_t), latency_store_comp);
}


double latency_store_calc_percentile(double percentile) {
	latency_store_sort();

	double tmp = round(percentile * current);

	double integral;
	double fractional;

	fractional = modf(tmp, &integral);
	if (fractional > 0) {
		uint64_t index = (uint64_t) ceil(tmp);

		return latencies[index];
	}
	else {
		uint64_t index = (uint64_t) integral;
		uint64_t v1 = latencies[index];
		uint64_t v2 = latencies[(index - 1)];

		return (v1 + v2) / 2;
	}
}
