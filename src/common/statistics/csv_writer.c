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

static gzFile lat_file = NULL;
static gzFile tp_file = NULL;
static gzFile tpr_file = NULL;

gzFile csv_write_open_file(const char *dir, const char *name, gru_status_t *status) {

	char *fullpath = gru_path_format(dir, name, status);

	if (!gru_path_rename(fullpath, status)) {
		gru_dealloc_string(&fullpath);
		return NULL;
	}

	gzFile f = gzopen(fullpath, "wb");
	gru_dealloc_string(&fullpath);

	if (!f) {
		gru_status_strerror(status, GRU_FAILURE, errno);
		return NULL;
	}

	return f;
}

static gzFile csv_write_initialize(char *directory, char *name, gru_status_t *status) {
	gzFile ret = NULL;

	if (!directory || !name) {
		gru_status_set(
			status, GRU_FAILURE, "Path and name are required for the CSV writer");

		return NULL;
	}

	ret = csv_write_open_file(directory, name, status);
	if (!ret) {
		return NULL;
	}

	return ret;
}

static bool csv_writer_finalize(gzFile file, gru_status_t *status) {
	if (gzclose(file) != 0) {
		gru_status_strerror(status, GRU_FAILURE, errno);
		return false;
	}

	return true;
}

static bool csv_writer_flush(gzFile file, gru_status_t *status) {
	if (gzflush(file, Z_SYNC_FLUSH) != 0) {
		gru_status_strerror(status, GRU_FAILURE, errno);

		return false;
	}

	return true;
}

static inline void csv_lat_writer_initialize_v1() {
	gzprintf(lat_file, "creation,latency\n");
}

bool csv_lat_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	if (!io_info) {
		gru_status_set(
			status, GRU_FAILURE, "The I/O information is required for the CSV writer");

		return false;
	}

	lat_file = csv_write_initialize(io_info->dest.location, io_info->dest.name, status);

	if (!lat_file) {
		return false;
	}

	switch (io_info->version) {
		case MPT_STATS_V1: {
			csv_lat_writer_initialize_v1();
			break;
		}
	}

	return csv_lat_writer_flush(status);
}

static bool csv_lat_writer_write_v1(const stat_latency_t *latency, gru_status_t *status) {
	char *str =
		gru_time_write_format(&latency->duration.start, "%Y-%m-%d %H:%M:%S", status);

	if (unlikely(!str)) {
		return false;
	}

	int64_t milli_latency = gru_time_to_milli(&latency->elapsed);
	uint32_t milli = (uint32_t) latency->duration.start.tv_usec / 1000;

	gzprintf(lat_file, "\"%s.%03" PRId32 "\",%" PRIu64 "\n", str, milli, milli_latency);

	gru_dealloc_string(&str);
	return true;
}

bool csv_lat_writer_write(const stat_latency_t *latency, gru_status_t *status) {
	switch (latency->version) {
		case MPT_STATS_V1: {
			return csv_lat_writer_write_v1(latency, status);
		}
	}

	return false;
}

bool csv_lat_writer_flush(gru_status_t *status) {
	return csv_writer_flush(lat_file, status);
}

bool csv_lat_writer_finalize(gru_status_t *status) {
	return csv_writer_finalize(lat_file, status);
}

static inline void csv_tp_writer_initialize_v1() {
	gzprintf(tp_file, "timestamp,count,rate\n");
}

bool csv_tp_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	if (!io_info) {
		gru_status_set(
			status, GRU_FAILURE, "The I/O information is required for the CSV writer");

		return false;
	}

	tp_file = csv_write_initialize(io_info->dest.location, io_info->dest.name, status);

	if (!tp_file) {
		return false;
	}

	switch (io_info->version) {
		case MPT_STATS_V1: {
			csv_tp_writer_initialize_v1();
			break;
		}
	}
	return csv_tp_writer_flush(status);
}

bool csv_tp_writer_write_v1(const stat_throughput_t *tp, gru_status_t *status) {
	char *str = gru_time_write_format(&tp->duration.end, "%Y-%m-%d %H:%M:%S", status);

	if (unlikely(!str)) {
		return false;
	}

	gzprintf(tp_file, "\"%s\",%" PRIu64 ",%.2f\n", str, tp->count, tp->rate);
	gru_dealloc_string(&str);
	return true;
}

bool csv_tp_writer_write(const stat_throughput_t *tp, gru_status_t *status) {
	switch (tp->version) {
		case MPT_STATS_V1: {
			return csv_tp_writer_write_v1(tp, status);
		}
	}

	return false;
}


bool csv_tp_writer_flush(gru_status_t *status) {
	return csv_writer_flush(tp_file, status);
}

bool csv_tp_writer_finalize(gru_status_t *status) {
	return csv_writer_finalize(tp_file, status);
}

static inline void csv_tpr_writer_initialize_v1() {
	gzprintf(tpr_file, "eta;ata\n");
}

bool csv_tpr_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	if (!io_info) {
		gru_status_set(
			status, GRU_FAILURE, "The I/O information is required for the CSV writer");

		return false;
	}

	tpr_file = csv_write_initialize(io_info->dest.location, io_info->dest.name, status);

	if (!tpr_file) {
		return false;
	}

	switch (io_info->version) {
	case MPT_STATS_V1: {
		csv_tpr_writer_initialize_v1();
		break;
	}
	}
	return csv_tpr_writer_flush(status);
}

bool csv_tpr_writer_write_v1(const stat_throughput_t *tp, gru_timestamp_t *eta, gru_status_t *status) {
	char *eta_str =
		gru_time_write_format(eta, "%Y-%m-%d %H:%M:%S", status);

	if (unlikely(!eta_str)) {
		return false;
	}

	char *ata_str =
		gru_time_write_format(&tp->duration.end, "%Y-%m-%d %H:%M:%S", status);

	if (unlikely(!ata_str)) {
		return false;
	}

	gzprintf(tpr_file, "\"%s.%" PRId32 "\",\"%s.%" PRIu32 "\"\n", eta_str, eta->tv_usec, ata_str, tp->duration.end.tv_usec);

	gru_dealloc_string(&eta_str);
	gru_dealloc_string(&ata_str);

	return true;
}

bool csv_tpr_writer_write(const stat_throughput_t *tp, gru_timestamp_t *eta, gru_status_t *status) {
	switch (tp->version) {
	case MPT_STATS_V1: {
		return csv_tpr_writer_write_v1(tp, eta, status);
	}
	}

	return false;
}

bool csv_tpr_writer_flush(gru_status_t *status) {
	return csv_writer_flush(tpr_file, status);
}

bool csv_tpr_writer_finalize(gru_status_t *status) {
	return csv_writer_finalize(tpr_file, status);
}

void csv_writer_latency_assign(latency_writer_t *writer) {
	writer->initialize = csv_lat_writer_initialize;
	writer->write = csv_lat_writer_write;
	writer->flush = csv_lat_writer_flush;
	writer->finalize = csv_lat_writer_finalize;
}

void csv_writer_throughput_assign(throughput_writer_t *writer) {
	writer->initialize = csv_tp_writer_initialize;
	writer->write = csv_tp_writer_write;
	writer->flush = csv_tp_writer_flush;
	writer->finalize = csv_tp_writer_finalize;
}

void csv_writer_tpr_assign(tpr_writer_t *writer) {
	writer->initialize = csv_tpr_writer_initialize;
	writer->write = csv_tpr_writer_write;
	writer->flush = csv_tpr_writer_flush;
	writer->finalize = csv_tpr_writer_finalize;
}