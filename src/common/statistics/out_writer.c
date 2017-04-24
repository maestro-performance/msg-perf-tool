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
#include "out_writer.h"

static gru_timestamp_t last_latency_duration;

bool out_lat_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	return true;
}


bool out_lat_writer_write(const stat_latency_t *latency, gru_status_t *status) {
	static suseconds_t last = 0;

	if ((latency->duration.start.tv_usec - 1) <= last) {
		return true;
	}

	last_latency_duration = latency->elapsed;
	return true;
}


bool out_lat_writer_flush(gru_status_t *status) {
	fflush(stdout);
	return true;
}


bool out_lat_writer_finalize(gru_status_t *status) {
	return true;
}


bool out_tp_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	return true;
}

bool out_tp_writer_write(const stat_throughput_t *tp, gru_status_t *status) {
	char *str = gru_time_write_format(&tp->duration.end, "%Y-%m-%d %H:%M:%S", 
		status);

	if (unlikely(!str)) {
		return false;
	}

	uint64_t milli_latency = gru_time_to_milli(&last_latency_duration);
	
	printf("%s\rSampling time: %s - Count: %" PRIu64 " msgs - Rate: %.2f msg/s - Last lat: %" PRIu64 
		"ms \r", 
		CLEAR_LINE, str, tp->count, tp->rate, milli_latency);

	gru_dealloc_string(&str);
	fflush(stdout);
	return true;
}

bool out_tp_writer_flush(gru_status_t *status) {
	fflush(stdout);
	return true;
}


bool out_tp_writer_finalize(gru_status_t *status) {
	printf("\n");
	return true;
}


/** 
 * Latency writer I/O assignment
 */
void out_writer_latency_assign(latency_writer_t *writer) {
	writer->initialize = out_lat_writer_initialize;
	writer->write = out_lat_writer_write;
	writer->flush = out_lat_writer_flush;
	writer->finalize = out_lat_writer_finalize;
}


/**
 * Throghput writer I/O assignment
 */
void out_writer_throughput_assign(throughput_writer_t *writer) {
	writer->initialize = out_tp_writer_initialize;
	writer->write = out_tp_writer_write;
	writer->flush = out_tp_writer_flush;
	writer->finalize = out_tp_writer_finalize;
}