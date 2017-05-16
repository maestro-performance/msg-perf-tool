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
#include "calculator.h"

void calc_latency(stat_latency_t *out, gru_timestamp_t start, gru_timestamp_t end) {
	out->duration.start = start;
	out->duration.end = end;

	timersub(&end, &start, &out->elapsed);
}

void calc_throughput(stat_throughput_t *out,
	gru_timestamp_t start,
	gru_timestamp_t end,
	uint64_t count) {
	out->duration.start = start;
	out->duration.end = end;
	out->count = count;

	uint64_t elapsed = gru_time_elapsed_secs(start, end);
	out->rate = ((double) count / elapsed);
}