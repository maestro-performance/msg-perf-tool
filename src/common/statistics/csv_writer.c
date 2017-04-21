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
#include "csv_writer.h"

static FILE *lat_file = NULL;
static FILE *tp_file = NULL;

static FILE *csv_write_initialize(char *directory, char *name, gru_status_t *status) {
	FILE *ret = NULL;

	if (!directory || !name) {
		gru_status_set(status, GRU_FAILURE, 
			"Path and name are required for the CSV writer");

		return NULL;
	}

	ret = gru_io_open_unique_file(directory, name, status);
	if (!ret) {
		return false;
	}

	return ret;
}

static bool csv_writer_finalize(FILE *file, gru_status_t *status) {
	if (fclose(file) != 0) {
		gru_status_strerror(status, GRU_FAILURE, errno);
		return false;
	}

	return true;
}

bool csv_writer_flush(FILE *file, gru_status_t *status) {
	if (fflush(file) != 0) {
		gru_status_strerror(status, GRU_FAILURE, errno);
		
		return false;
	}

	return true;
}

bool csv_lat_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	if (!io_info) {
		gru_status_set(status, GRU_FAILURE, 
			"The I/O information is required for the CSV writer");

		return false;
	}

	lat_file = csv_write_initialize(io_info->dest.location, io_info->dest.name, status);

	if (!lat_file) {
		return false;
	}
	
	fprintf(lat_file, "creation;latency\n");
	return csv_lat_writer_flush(status);
}



bool csv_lat_writer_write(const stat_latency_t *latency, gru_status_t *status) {
	char *str = gru_time_write_format(&latency->duration.start, "%Y-%m-%d %H:%M:%S", 
		status);

	if (unlikely(!str)) {
		return false;
	}
	
	uint64_t milli_latency = gru_time_to_milli(&latency->elapsed);
	uint32_t milli = (uint32_t) latency->duration.start.tv_usec / 1000;

	fprintf(lat_file, "%s.%03" PRId32 ";%" PRIu64 "\n", str, milli, milli_latency);

	gru_dealloc_string(&str);
	return true;
}


bool csv_lat_writer_flush(gru_status_t *status) {
	return csv_writer_flush(lat_file, status);
}


bool csv_lat_writer_finalize(gru_status_t *status) {
	return csv_writer_finalize(lat_file, status);
}


bool csv_tp_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	if (!io_info) {
		gru_status_set(status, GRU_FAILURE, 
			"The I/O information is required for the CSV writer");

		return false;
	}

	tp_file = csv_write_initialize(io_info->dest.location, io_info->dest.name, status);

	if (!tp_file) {
		return false;
	}
	
	fprintf(tp_file, "timestamp;count;rate\n");
	return csv_tp_writer_flush(status);
}


int csv_tp_writer_write(const stat_throughput_t *tp, gru_status_t *status) {
	char *str = gru_time_write_format(&tp->duration.end, "%Y-%m-%d %H:%M:%S", 
		status);

	if (unlikely(!str)) {
		return false;
	}

	fprintf(tp_file, "%s;%" PRIu64 ";%.2f\n", str, tp->count, tp->rate);
	gru_dealloc_string(&str);
	return true;
}


bool csv_tp_writer_flush(gru_status_t *status) {
	return csv_writer_flush(tp_file, status);
}


bool csv_tp_writer_finalize(gru_status_t *status) {
	return csv_writer_finalize(tp_file, status);
}


bool csv_writer_latency_assign(latency_writer_t *writer) {
	writer->initialize = csv_lat_writer_initialize;
	writer->write = csv_lat_writer_write;
	writer->flush = csv_lat_writer_flush;
	writer->finalize = csv_lat_writer_finalize;
}

bool csv_writer_throughput_assign(throughput_writer_t *writer) {
	writer->initialize = csv_tp_writer_initialize;
	writer->write = csv_tp_writer_write;
	writer->flush = csv_tp_writer_flush;
	writer->finalize = csv_tp_writer_finalize;
}